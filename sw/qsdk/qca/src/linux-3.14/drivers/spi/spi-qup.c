/*
 * Copyright (c) 2008-2016, The Linux foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License rev 2 and
 * only rev 2 as published by the free Software foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or fITNESS fOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>

#define QUP_CONFIG			0x0000
#define QUP_STATE			0x0004
#define QUP_IO_M_MODES			0x0008
#define QUP_SW_RESET			0x000c
#define QUP_OPERATIONAL			0x0018
#define QUP_ERROR_FLAGS			0x001c
#define QUP_ERROR_FLAGS_EN		0x0020
#define QUP_OPERATIONAL_MASK		0x0028
#define QUP_HW_VERSION			0x0030
#define QUP_MX_OUTPUT_CNT		0x0100
#define QUP_OUTPUT_FIFO			0x0110
#define QUP_MX_WRITE_CNT		0x0150
#define QUP_MX_INPUT_CNT		0x0200
#define QUP_MX_READ_CNT			0x0208
#define QUP_INPUT_FIFO			0x0218

#define SPI_CONFIG			0x0300
#define SPI_IO_CONTROL			0x0304
#define SPI_ERROR_FLAGS			0x0308
#define SPI_ERROR_FLAGS_EN		0x030c

/* QUP_CONFIG fields */
#define QUP_CONFIG_SPI_MODE		(1 << 8)
#define QUP_CONFIG_CLOCK_AUTO_GATE	BIT(13)
#define QUP_CONFIG_NO_INPUT		BIT(7)
#define QUP_CONFIG_NO_OUTPUT		BIT(6)
#define QUP_CONFIG_N			0x001f

/* QUP_STATE fields */
#define QUP_STATE_VALID			BIT(2)
#define QUP_STATE_RESET			0
#define QUP_STATE_RUN			1
#define QUP_STATE_PAUSE			3
#define QUP_STATE_MASK			3
#define QUP_STATE_CLEAR			2

#define QUP_HW_VERSION_2_1_1		0x20010001

/* QUP_IO_M_MODES fields */
#define QUP_IO_M_PACK_EN		BIT(15)
#define QUP_IO_M_UNPACK_EN		BIT(14)
#define QUP_IO_M_INPUT_MODE_MASK_SHIFT	12
#define QUP_IO_M_OUTPUT_MODE_MASK_SHIFT	10
#define QUP_IO_M_INPUT_MODE_MASK	(3 << QUP_IO_M_INPUT_MODE_MASK_SHIFT)
#define QUP_IO_M_OUTPUT_MODE_MASK	(3 << QUP_IO_M_OUTPUT_MODE_MASK_SHIFT)

#define QUP_IO_M_OUTPUT_BLOCK_SIZE(x)	(((x) & (0x03 << 0)) >> 0)
#define QUP_IO_M_OUTPUT_FIFO_SIZE(x)	(((x) & (0x07 << 2)) >> 2)
#define QUP_IO_M_INPUT_BLOCK_SIZE(x)	(((x) & (0x03 << 5)) >> 5)
#define QUP_IO_M_INPUT_FIFO_SIZE(x)	(((x) & (0x07 << 7)) >> 7)

#define QUP_IO_M_MODE_FIFO		0
#define QUP_IO_M_MODE_BLOCK		1
#define QUP_IO_M_MODE_DMOV		2
#define QUP_IO_M_MODE_BAM		3

/* QUP_OPERATIONAL fields */
#define QUP_OP_IN_BLOCK_READ_REQ	BIT(13)
#define QUP_OP_OUT_BLOCK_WRITE_REQ	BIT(12)
#define QUP_OP_MAX_INPUT_DONE_FLAG	BIT(11)
#define QUP_OP_MAX_OUTPUT_DONE_FLAG	BIT(10)
#define QUP_OP_IN_SERVICE_FLAG		BIT(9)
#define QUP_OP_OUT_SERVICE_FLAG		BIT(8)
#define QUP_OP_IN_FIFO_FULL		BIT(7)
#define QUP_OP_OUT_FIFO_FULL		BIT(6)
#define QUP_OP_IN_FIFO_NOT_EMPTY	BIT(5)
#define QUP_OP_OUT_FIFO_NOT_EMPTY	BIT(4)

/* QUP_ERROR_FLAGS and QUP_ERROR_FLAGS_EN fields */
#define QUP_ERROR_OUTPUT_OVER_RUN	BIT(5)
#define QUP_ERROR_INPUT_UNDER_RUN	BIT(4)
#define QUP_ERROR_OUTPUT_UNDER_RUN	BIT(3)
#define QUP_ERROR_INPUT_OVER_RUN	BIT(2)

/* SPI_CONFIG fields */
#define SPI_CONFIG_HS_MODE		BIT(10)
#define SPI_CONFIG_INPUT_FIRST		BIT(9)
#define SPI_CONFIG_LOOPBACK		BIT(8)

/* SPI_IO_CONTROL fields */
#define SPI_IO_C_FORCE_CS		BIT(11)
#define SPI_IO_C_CLK_IDLE_HIGH		BIT(10)
#define SPI_IO_C_MX_CS_MODE		BIT(8)
#define SPI_IO_C_CS_N_POLARITY_0	BIT(4)
#define SPI_IO_C_CS_SELECT(x)		(((x) & 3) << 2)
#define SPI_IO_C_CS_SELECT_MASK		0x000c
#define SPI_IO_C_TRISTATE_CS		BIT(1)
#define SPI_IO_C_NO_TRI_STATE		BIT(0)

/* SPI_ERROR_FLAGS and SPI_ERROR_FLAGS_EN fields */
#define SPI_ERROR_CLK_OVER_RUN		BIT(1)
#define SPI_ERROR_CLK_UNDER_RUN		BIT(0)

#define SPI_NUM_CHIPSELECTS		4

#define SPI_MAX_XFER			(SZ_64K - 64)

/* high speed mode is when bus rate is greater then 26MHz */
#define SPI_HS_MIN_RATE			26000000
#define SPI_MAX_RATE			50000000

#define SPI_DELAY_THRESHOLD		1
#define SPI_DELAY_RETRY			10

struct spi_qup {
	void __iomem		*base;
	struct device		*dev;
	struct clk		*cclk;	/* core clock */
	struct clk		*iclk;	/* interface clock */
	int			irq;
	spinlock_t		lock;

	int			in_fifo_sz;
	int			out_fifo_sz;
	int			in_blk_sz;
	int			out_blk_sz;

	struct spi_transfer	*xfer;
	struct completion	done;
	int			error;
	int			w_size;	/* bytes per SPI word */
	int			tx_bytes;
	int			rx_bytes;
	int			qup_v1;
	int			use_dma;

	struct dma_chan		*rx_chan;
	struct dma_slave_config	rx_conf;
	struct dma_chan		*tx_chan;
	struct dma_slave_config tx_conf;
	void			*dummy;
	dma_addr_t		dummy_phys;
	atomic_t		dma_outstanding;
	int			mode;
};


static inline bool spi_qup_is_valid_state(struct spi_qup *controller)
{
	u32 opstate = readl_relaxed(controller->base + QUP_STATE);

	return opstate & QUP_STATE_VALID;
}

static int spi_qup_set_state(struct spi_qup *controller, u32 state)
{
	unsigned long loop;
	u32 cur_state;

	loop = 0;
	while (!spi_qup_is_valid_state(controller)) {

		usleep_range(SPI_DELAY_THRESHOLD, SPI_DELAY_THRESHOLD * 2);

		if (++loop > SPI_DELAY_RETRY)
			return -EIO;
	}

	if (loop)
		dev_dbg(controller->dev, "invalid state for %ld,us %d\n",
			loop, state);

	cur_state = readl_relaxed(controller->base + QUP_STATE);
	/*
	 * Per spec: for PAUSE_STATE to RESET_STATE, two writes
	 * of (b10) are required
	 */
	if (((cur_state & QUP_STATE_MASK) == QUP_STATE_PAUSE) &&
	    (state == QUP_STATE_RESET)) {
		writel_relaxed(QUP_STATE_CLEAR, controller->base + QUP_STATE);
		writel_relaxed(QUP_STATE_CLEAR, controller->base + QUP_STATE);
	} else {
		cur_state &= ~QUP_STATE_MASK;
		cur_state |= state;
		writel_relaxed(cur_state, controller->base + QUP_STATE);
	}

	loop = 0;
	while (!spi_qup_is_valid_state(controller)) {

		usleep_range(SPI_DELAY_THRESHOLD, SPI_DELAY_THRESHOLD * 2);

		if (++loop > SPI_DELAY_RETRY)
			return -EIO;
	}

	return 0;
}

static void spi_qup_fill_read_buffer(struct spi_qup *controller,
	struct spi_transfer *xfer, u32 data)
{
	u8 *rx_buf = xfer->rx_buf;
	int idx, shift;

	if (rx_buf)
		for (idx = 0; idx < controller->w_size; idx++) {
			/*
			 * The data format depends on bytes per SPI word:
			 *  4 bytes: 0x12345678
			 *  2 bytes: 0x00001234
			 *  1 byte : 0x00000012
			 */
			shift = BITS_PER_BYTE;
			shift *= (controller->w_size - idx - 1);
			rx_buf[controller->rx_bytes + idx] = data >> shift;
		}

	controller->rx_bytes += controller->w_size;
}

static void spi_qup_prepare_write_data(struct spi_qup *controller,
	struct spi_transfer *xfer, u32 *data)
{
	const u8 *tx_buf = xfer->tx_buf;
	u32 val;
	int idx;

	*data = 0;

	if (tx_buf)
		for (idx = 0; idx < controller->w_size; idx++) {
			val = tx_buf[controller->tx_bytes + idx];
			*data |= val << (BITS_PER_BYTE * (3 - idx));
		}

	controller->tx_bytes += controller->w_size;
}

static void spi_qup_fifo_read(struct spi_qup *controller,
			    struct spi_transfer *xfer)
{
	u32 data;

	/* clear service request */
	writel_relaxed(QUP_OP_IN_SERVICE_FLAG,
			controller->base + QUP_OPERATIONAL);

	while (controller->rx_bytes < xfer->len) {
		if (!(readl_relaxed(controller->base + QUP_OPERATIONAL) &
		    QUP_OP_IN_FIFO_NOT_EMPTY))
			break;

		data = readl_relaxed(controller->base + QUP_INPUT_FIFO);

		spi_qup_fill_read_buffer(controller, xfer, data);
	}
}

static void spi_qup_fifo_write(struct spi_qup *controller,
	struct spi_transfer *xfer)
{
	u32 data;

	/* clear service request */
	writel_relaxed(QUP_OP_OUT_SERVICE_FLAG,
		controller->base + QUP_OPERATIONAL);

	while (controller->tx_bytes < xfer->len) {

		if (readl_relaxed(controller->base + QUP_OPERATIONAL) &
				QUP_OP_OUT_FIFO_FULL)
			break;

		spi_qup_prepare_write_data(controller, xfer, &data);
		writel_relaxed(data, controller->base + QUP_OUTPUT_FIFO);

	}
}

static void spi_qup_block_read(struct spi_qup *controller,
	struct spi_transfer *xfer, u32 *opflags)
{
	u32 data;
	u32 reads_per_blk = controller->in_blk_sz >> 2;
	u32 num_words = (xfer->len - controller->rx_bytes) / controller->w_size;
	int i;

	do {
		/* ACK by clearing service flag */
		writel_relaxed(QUP_OP_IN_SERVICE_FLAG,
			controller->base + QUP_OPERATIONAL);

		/* transfer up to a block size of data in a single pass */
		for (i = 0; num_words && i < reads_per_blk; i++, num_words--) {

			/* read data and fill up rx buffer */
			data = readl_relaxed(controller->base + QUP_INPUT_FIFO);
			spi_qup_fill_read_buffer(controller, xfer, data);
		}

		/* check to see if next block is ready */
		if (!(readl_relaxed(controller->base + QUP_OPERATIONAL) &
			QUP_OP_IN_BLOCK_READ_REQ))
			break;

	} while (num_words);

	/*
	 * Due to extra stickiness of the QUP_OP_IN_SERVICE_FLAG during block
	 * reads, it has to be cleared again at the very end.  However, be sure
	 * to refresh opflags value because MAX_INPUT_DONE_FLAG may now be
	 * present and this is used to determine if transaction is complete
	 */
	*opflags = readl_relaxed(controller->base + QUP_OPERATIONAL);
	if (*opflags & QUP_OP_MAX_INPUT_DONE_FLAG)
		writel_relaxed(QUP_OP_IN_SERVICE_FLAG,
			controller->base + QUP_OPERATIONAL);

}

static void spi_qup_block_write(struct spi_qup *controller,
	struct spi_transfer *xfer)
{
	u32 data;
	u32 writes_per_blk = controller->out_blk_sz >> 2;
	u32 num_words = (xfer->len - controller->tx_bytes) / controller->w_size;
	int i;

	do {
		/* ACK by clearing service flag */
		writel_relaxed(QUP_OP_OUT_SERVICE_FLAG,
			controller->base + QUP_OPERATIONAL);

		/* transfer up to a block size of data in a single pass */
		for (i = 0; num_words && i < writes_per_blk; i++, num_words--) {

			/* swizzle the bytes for output and write out */
			spi_qup_prepare_write_data(controller, xfer, &data);
			writel_relaxed(data,
				controller->base + QUP_OUTPUT_FIFO);
		}

		/* check to see if next block is ready */
		if (!(readl_relaxed(controller->base + QUP_OPERATIONAL) &
			QUP_OP_OUT_BLOCK_WRITE_REQ))
			break;

	} while (num_words);
}

static void qup_dma_callback(void *data)
{
	struct spi_qup *controller = data;

	if (atomic_dec_and_test(&controller->dma_outstanding))
		complete(&controller->done);
}

static int spi_qup_do_dma(struct spi_qup *controller, struct spi_transfer *xfer)
{
	struct dma_async_tx_descriptor *rxd, *txd;
	dma_cookie_t rx_cookie, tx_cookie;
	u32 xfer_len, rx_align = 0, tx_align = 0, n_words;
	struct scatterlist tx_sg[2], rx_sg[2];
	int ret = 0;
	u32 bytes_to_xfer = xfer->len;
	u32 offset = 0;
	u32 rx_nents = 0, tx_nents = 0;
	dma_addr_t rx_dma = 0, tx_dma = 0;


	if (xfer->rx_buf) {
		rx_dma = dma_map_single(controller->dev, xfer->rx_buf,
			xfer->len, DMA_FROM_DEVICE);

		if (dma_mapping_error(controller->dev, rx_dma)) {
			ret = -ENOMEM;
			return ret;
		}
	}

	if (xfer->tx_buf) {
		/* check to see if we need dummy buffer for leftover bytes */
		tx_align = xfer->len % controller->out_blk_sz;

		if (tx_align) {
			memset(controller->dummy + controller->in_blk_sz + tx_align,
				0, controller->out_blk_sz - tx_align);
			memcpy(controller->dummy + controller->in_blk_sz,
				xfer->tx_buf + xfer->len - tx_align, tx_align);
		}

		tx_dma = dma_map_single(controller->dev,
			(void *)xfer->tx_buf, xfer->len, DMA_TO_DEVICE);

		if (dma_mapping_error(controller->dev, tx_dma)) {
			ret = -ENOMEM;
			goto err_map_tx;
		}
	}

	atomic_set(&controller->dma_outstanding, 0);

	while (bytes_to_xfer > 0) {
		xfer_len = min_t(u32, bytes_to_xfer, SPI_MAX_XFER);
		n_words = DIV_ROUND_UP(xfer_len, controller->w_size);

		if (xfer_len == bytes_to_xfer) {
			tx_align = xfer_len % controller->out_blk_sz;
			rx_align = xfer_len % controller->in_blk_sz;
		} else {
			tx_align = 0;
			rx_align = 0;
		}

		/* write out current word count to controller */
		writel_relaxed(n_words, controller->base + QUP_MX_INPUT_CNT);
		writel_relaxed(n_words, controller->base + QUP_MX_OUTPUT_CNT);

		reinit_completion(&controller->done);

		if (xfer->tx_buf) {
			if (tx_align)
				tx_nents = 2;
			else
				tx_nents = 1;

			/* initialize scatterlists */
			sg_init_table(tx_sg, tx_nents);
			sg_dma_len(&tx_sg[0]) = xfer_len - tx_align;
			sg_dma_address(&tx_sg[0]) = tx_dma + offset;

			/* account for non block size transfer */
			if (tx_align) {
				sg_dma_len(&tx_sg[1]) = controller->out_blk_sz;
				sg_dma_address(&tx_sg[1]) =
					controller->dummy_phys +
						controller->in_blk_sz;
			}

			txd = dmaengine_prep_slave_sg(controller->tx_chan,
					tx_sg, tx_nents, DMA_MEM_TO_DEV, 0);
			if (!txd) {
				ret = -ENOMEM;
				goto err_unmap;
			}

			atomic_inc(&controller->dma_outstanding);

			txd->callback = qup_dma_callback;
			txd->callback_param = controller;

			tx_cookie = dmaengine_submit(txd);

			dma_async_issue_pending(controller->tx_chan);
		}

		if (xfer->rx_buf) {
			if (rx_align)
				rx_nents = 2;
			else
				rx_nents = 1;

			/* initialize scatterlists */
			sg_init_table(rx_sg, rx_nents);
			sg_dma_address(&rx_sg[0]) = rx_dma + offset;
			sg_dma_len(&rx_sg[0]) = xfer_len - rx_align;

			/* account for non block size transfer */
			if (rx_align) {
				sg_dma_len(&rx_sg[1]) = controller->in_blk_sz;
				sg_dma_address(&rx_sg[1]) =
					controller->dummy_phys;
			}

			rxd = dmaengine_prep_slave_sg(controller->rx_chan,
					rx_sg, rx_nents, DMA_DEV_TO_MEM, 0);
			if (!rxd) {
				ret = -ENOMEM;
				goto err_unmap;
			}

			atomic_inc(&controller->dma_outstanding);

			rxd->callback = qup_dma_callback;
			rxd->callback_param = controller;

			rx_cookie = dmaengine_submit(rxd);

			dma_async_issue_pending(controller->rx_chan);
		}

		if (spi_qup_set_state(controller, QUP_STATE_RUN)) {
			dev_warn(controller->dev, "cannot set EXECUTE state\n");
			goto err_unmap;
		}

		if (!wait_for_completion_timeout(&controller->done,
			msecs_to_jiffies(1000))) {
			ret = -ETIMEDOUT;

			/* clear out all the DMA transactions */
			if (xfer->tx_buf)
				dmaengine_terminate_all(controller->tx_chan);
			if (xfer->rx_buf)
				dmaengine_terminate_all(controller->rx_chan);

			goto err_unmap;
		}

		/* adjust remaining bytes to transfer */
		bytes_to_xfer -= xfer_len;
		offset += xfer_len;


		/* reset mini-core state so we can program next transaction */
		if (spi_qup_set_state(controller, QUP_STATE_RESET)) {
			dev_err(controller->dev, "cannot set RESET state\n");
			goto err_unmap;
		}
	}

	ret = 0;

err_unmap:
	if (xfer->tx_buf)
		dma_unmap_single(controller->dev, tx_dma, xfer->len,
					DMA_TO_DEVICE);
err_map_tx:
	if (xfer->rx_buf) {
		dma_unmap_single(controller->dev, rx_dma, xfer->len,
					DMA_FROM_DEVICE);

		if (rx_align)
			memcpy(xfer->rx_buf + xfer->len - rx_align,
				controller->dummy, rx_align);
	}

	return ret;
}

static irqreturn_t spi_qup_qup_irq(int irq, void *dev_id)
{
	struct spi_qup *controller = dev_id;
	struct spi_transfer *xfer;
	u32 opflags, qup_err, spi_err;
	unsigned long flags;
	int error = 0;

	spin_lock_irqsave(&controller->lock, flags);
	xfer = controller->xfer;
	controller->xfer = NULL;
	spin_unlock_irqrestore(&controller->lock, flags);

	qup_err = readl_relaxed(controller->base + QUP_ERROR_FLAGS);
	spi_err = readl_relaxed(controller->base + SPI_ERROR_FLAGS);
	opflags = readl_relaxed(controller->base + QUP_OPERATIONAL);

	writel_relaxed(qup_err, controller->base + QUP_ERROR_FLAGS);
	writel_relaxed(spi_err, controller->base + SPI_ERROR_FLAGS);

	if (!xfer) {
		writel_relaxed(opflags, controller->base + QUP_OPERATIONAL);
		dev_err_ratelimited(controller->dev, "unexpected irq %08x %08x %08x\n",
				    qup_err, spi_err, opflags);
		return IRQ_HANDLED;
	}

	if (qup_err) {
		if (qup_err & QUP_ERROR_OUTPUT_OVER_RUN)
			dev_warn(controller->dev, "OUTPUT_OVER_RUN\n");
		if (qup_err & QUP_ERROR_INPUT_UNDER_RUN)
			dev_warn(controller->dev, "INPUT_UNDER_RUN\n");
		if (qup_err & QUP_ERROR_OUTPUT_UNDER_RUN)
			dev_warn(controller->dev, "OUTPUT_UNDER_RUN\n");
		if (qup_err & QUP_ERROR_INPUT_OVER_RUN)
			dev_warn(controller->dev, "INPUT_OVER_RUN\n");

		error = -EIO;
	}

	if (spi_err) {
		if (spi_err & SPI_ERROR_CLK_OVER_RUN)
			dev_warn(controller->dev, "CLK_OVER_RUN\n");
		if (spi_err & SPI_ERROR_CLK_UNDER_RUN)
			dev_warn(controller->dev, "CLK_UNDER_RUN\n");

		error = -EIO;
	}

	if (controller->use_dma) {
		writel_relaxed(opflags, controller->base + QUP_OPERATIONAL);
	} else
	{
		if (opflags & QUP_OP_IN_SERVICE_FLAG) {
			if (opflags & QUP_OP_IN_BLOCK_READ_REQ)
				spi_qup_block_read(controller, xfer, &opflags);
			else
				spi_qup_fifo_read(controller, xfer);
		}

		if (opflags & QUP_OP_OUT_SERVICE_FLAG) {
			if (opflags & QUP_OP_OUT_BLOCK_WRITE_REQ)
				spi_qup_block_write(controller, xfer);
			else
				spi_qup_fifo_write(controller, xfer);
		}
	}

	spin_lock_irqsave(&controller->lock, flags);
	controller->error = error;
	controller->xfer = xfer;
	spin_unlock_irqrestore(&controller->lock, flags);

	if ((controller->rx_bytes == xfer->len &&
		(opflags & QUP_OP_MAX_INPUT_DONE_FLAG)) || error) {
		complete(&controller->done);
	} else if (controller->rx_bytes == xfer->len &&
		   !controller->use_dma && xfer->len >= 0x10000) {
		/*
		 * xxx_COUNT registers are 16 bits wide. Hence, the
		 * maximum count value that can be programmed in them is
		 * 0xffff. If the transfer length is greater than 0xffff,
		 * the registers get set to zero, implying infinite read
		 * mode. In these cases, the QUP_OP_MAX_INPUT_DONE_FLAG
		 * will not get set. Hence, terminate here if we have Rx-ed
		 * enough no. of bytes.
		 */
		complete(&controller->done);
	}

	return IRQ_HANDLED;
}

/* set clock freq ... bits per word */
static int spi_qup_io_config(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct spi_qup *controller = spi_master_get_devdata(spi->master);
	u32 config, iomode;
	int ret, n_words, w_size;
	size_t dma_align = dma_get_cache_alignment();
	u32 dma_available = 0;

	if (spi->mode & SPI_LOOP && xfer->len > controller->in_fifo_sz) {
		dev_err(controller->dev, "too big size for loopback %d > %d\n",
			xfer->len, controller->in_fifo_sz);
		return -EIO;
	}

	ret = clk_set_rate(controller->cclk, xfer->speed_hz);
	if (ret) {
		dev_err(controller->dev, "fail to set frequency %d",
			xfer->speed_hz);
		return -EIO;
	}

	if (spi_qup_set_state(controller, QUP_STATE_RESET)) {
		dev_err(controller->dev, "cannot set RESET state\n");
		return -EIO;
	}

	w_size = 4;
	if (xfer->bits_per_word <= 8)
		w_size = 1;
	else if (xfer->bits_per_word <= 16)
		w_size = 2;

	n_words = xfer->len / w_size;
	controller->w_size = w_size;

	if (controller->rx_chan &&
		IS_ALIGNED((size_t)xfer->tx_buf, dma_align) &&
		IS_ALIGNED((size_t)xfer->rx_buf, dma_align) &&
		!is_vmalloc_addr(xfer->tx_buf) &&
		!is_vmalloc_addr(xfer->rx_buf) &&
		(xfer->len > 3*controller->in_blk_sz))
		dma_available = 1;

	if (n_words <= (controller->in_fifo_sz / sizeof(u32))) {
		controller->mode = QUP_IO_M_MODE_FIFO;
		writel_relaxed(n_words, controller->base + QUP_MX_READ_CNT);
		writel_relaxed(n_words, controller->base + QUP_MX_WRITE_CNT);
		/* must be zero for FIFO */
		writel_relaxed(0, controller->base + QUP_MX_INPUT_CNT);
		writel_relaxed(0, controller->base + QUP_MX_OUTPUT_CNT);
		controller->use_dma = 0;
	} else if (!dma_available) {
		controller->mode = QUP_IO_M_MODE_BLOCK;
		writel_relaxed(n_words, controller->base + QUP_MX_INPUT_CNT);
		writel_relaxed(n_words, controller->base + QUP_MX_OUTPUT_CNT);
		/* must be zero for BLOCK and BAM */
		writel_relaxed(0, controller->base + QUP_MX_READ_CNT);
		writel_relaxed(0, controller->base + QUP_MX_WRITE_CNT);
		controller->use_dma = 0;
	} else {
		controller->mode = QUP_IO_M_MODE_DMOV;
		writel_relaxed(0, controller->base + QUP_MX_READ_CNT);
		writel_relaxed(0, controller->base + QUP_MX_WRITE_CNT);
		controller->use_dma = 1;
	}

	iomode = readl_relaxed(controller->base + QUP_IO_M_MODES);
	/* Set input and output transfer mode */
	iomode &= ~(QUP_IO_M_INPUT_MODE_MASK | QUP_IO_M_OUTPUT_MODE_MASK);

	if (!controller->use_dma)
		iomode &= ~(QUP_IO_M_PACK_EN | QUP_IO_M_UNPACK_EN);
	else
		iomode |= QUP_IO_M_PACK_EN | QUP_IO_M_UNPACK_EN;

	iomode |= (controller->mode << QUP_IO_M_OUTPUT_MODE_MASK_SHIFT);
	iomode |= (controller->mode << QUP_IO_M_INPUT_MODE_MASK_SHIFT);

	writel_relaxed(iomode, controller->base + QUP_IO_M_MODES);

	config = readl_relaxed(controller->base + SPI_CONFIG);

	if (spi->mode & SPI_LOOP)
		config |= SPI_CONFIG_LOOPBACK;
	else
		config &= ~SPI_CONFIG_LOOPBACK;

	if (spi->mode & SPI_CPHA)
		config &= ~SPI_CONFIG_INPUT_FIRST;
	else
		config |= SPI_CONFIG_INPUT_FIRST;

	/*
	 * HS_MODE improves signal stability for spi-clk high rates,
	 * but is invalid in loop back mode.
	 */
	if ((xfer->speed_hz >= SPI_HS_MIN_RATE) && !(spi->mode & SPI_LOOP))
		config |= SPI_CONFIG_HS_MODE;
	else
		config &= ~SPI_CONFIG_HS_MODE;

	writel_relaxed(config, controller->base + SPI_CONFIG);

	config = readl_relaxed(controller->base + QUP_CONFIG);
	config &= ~(QUP_CONFIG_NO_INPUT | QUP_CONFIG_NO_OUTPUT | QUP_CONFIG_N);
	config |= xfer->bits_per_word - 1;
	config |= QUP_CONFIG_SPI_MODE;

	if (controller->use_dma) {
		if (!xfer->tx_buf)
			config |= QUP_CONFIG_NO_OUTPUT;
		if (!xfer->rx_buf)
			config |= QUP_CONFIG_NO_INPUT;
	}

	writel_relaxed(config, controller->base + QUP_CONFIG);

	/* only write to OPERATIONAL_MASK when register is present */
	if (!controller->qup_v1)
		writel_relaxed(0, controller->base + QUP_OPERATIONAL_MASK);

	return 0;
}

static int spi_qup_transfer_one(struct spi_master *master,
			      struct spi_device *spi,
			      struct spi_transfer *xfer)
{
	struct spi_qup *controller = spi_master_get_devdata(master);
	unsigned long timeout, flags;
	int ret = -EIO;

	ret = spi_qup_io_config(spi, xfer);
	if (ret)
		return ret;

	timeout = DIV_ROUND_UP(xfer->speed_hz, MSEC_PER_SEC);
	timeout = DIV_ROUND_UP(xfer->len * 8, timeout);
	timeout = 100 * msecs_to_jiffies(timeout);

	reinit_completion(&controller->done);

	spin_lock_irqsave(&controller->lock, flags);
	controller->xfer     = xfer;
	controller->error    = 0;
	controller->rx_bytes = 0;
	controller->tx_bytes = 0;
	spin_unlock_irqrestore(&controller->lock, flags);

	if (controller->use_dma) {
		ret = spi_qup_do_dma(controller, xfer);
	} else {
		if (spi_qup_set_state(controller, QUP_STATE_RUN)) {
			dev_warn(controller->dev, "cannot set RUN state\n");
			goto exit;
		}

		if (spi_qup_set_state(controller, QUP_STATE_PAUSE)) {
			dev_warn(controller->dev, "cannot set PAUSE state\n");
			goto exit;
		}

		if (controller->mode == QUP_IO_M_MODE_FIFO)
			spi_qup_fifo_write(controller, xfer);

		if (spi_qup_set_state(controller, QUP_STATE_RUN)) {
			dev_warn(controller->dev, "cannot set EXECUTE state\n");
			goto exit;
		}

		if (!ret && !wait_for_completion_timeout(&controller->done,
				timeout))
			ret = -ETIMEDOUT;
		}
exit:

	spi_qup_set_state(controller, QUP_STATE_RESET);
	spin_lock_irqsave(&controller->lock, flags);
	controller->xfer = NULL;
	if (!ret)
		ret = controller->error;
	spin_unlock_irqrestore(&controller->lock, flags);

	return ret;
}

static int spi_qup_setup(struct spi_device *spi)
{
	if (!gpio_is_valid(spi->cs_gpio))
		return 0;

	if (spi->mode & SPI_CS_HIGH)
		gpio_set_value(spi->cs_gpio, 0);
	else
		gpio_set_value(spi->cs_gpio, 1);

	udelay(10);

	return 0;
}

static int spi_qup_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct clk *iclk, *cclk;
	struct spi_qup *controller;
	struct resource *res;
	struct device *dev;
	void __iomem *base;
	u32 max_freq, iomode;
	int ret, irq, size;

	dev = &pdev->dev;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL)
		return -EINVAL;

	base = devm_ioremap_resource(dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	cclk = devm_clk_get(dev, "core");
	if (IS_ERR(cclk))
		return PTR_ERR(cclk);

	iclk = devm_clk_get(dev, "iface");
	if (IS_ERR(iclk))
		return PTR_ERR(iclk);

	/* This is optional parameter */
	if (of_property_read_u32(dev->of_node, "spi-max-frequency", &max_freq))
		max_freq = SPI_MAX_RATE;

	if (!max_freq || max_freq > SPI_MAX_RATE) {
		dev_err(dev, "invalid clock frequency %d\n", max_freq);
		return -ENXIO;
	}

	ret = clk_prepare_enable(cclk);
	if (ret) {
		dev_err(dev, "cannot enable core clock\n");
		return ret;
	}

	ret = clk_prepare_enable(iclk);
	if (ret) {
		clk_disable_unprepare(cclk);
		dev_err(dev, "cannot enable iface clock\n");
		return ret;
	}

	master = spi_alloc_master(dev, sizeof(struct spi_qup));
	if (!master) {
		clk_disable_unprepare(cclk);
		clk_disable_unprepare(iclk);
		dev_err(dev, "cannot allocate master\n");
		return -ENOMEM;
	}

	/* use num-cs unless not present or out of range */
	if (of_property_read_u16(dev->of_node, "num-cs",
			&master->num_chipselect) ||
			(master->num_chipselect > SPI_NUM_CHIPSELECTS))
		master->num_chipselect = SPI_NUM_CHIPSELECTS;

	master->bus_num = pdev->id;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_LOOP;
	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
	master->max_speed_hz = max_freq;
	master->transfer_one = spi_qup_transfer_one;
	master->setup = spi_qup_setup;
	master->dev.of_node = pdev->dev.of_node;
	master->auto_runtime_pm = true;
	master->dma_alignment = dma_get_cache_alignment();

	platform_set_drvdata(pdev, master);

	controller = spi_master_get_devdata(master);

	controller->dev = dev;
	controller->base = base;
	controller->iclk = iclk;
	controller->cclk = cclk;
	controller->irq = irq;

	/* set v1 flag if device is version 1 */
	if (of_device_is_compatible(dev->of_node, "qcom,spi-qup-v1.1.1"))
		controller->qup_v1 = 1;

	spin_lock_init(&controller->lock);
	init_completion(&controller->done);

	iomode = readl_relaxed(base + QUP_IO_M_MODES);

	size = QUP_IO_M_OUTPUT_BLOCK_SIZE(iomode);
	if (size)
		controller->out_blk_sz = size * 16;
	else
		controller->out_blk_sz = 4;

	size = QUP_IO_M_INPUT_BLOCK_SIZE(iomode);
	if (size)
		controller->in_blk_sz = size * 16;
	else
		controller->in_blk_sz = 4;

	size = QUP_IO_M_OUTPUT_FIFO_SIZE(iomode);
	controller->out_fifo_sz = controller->out_blk_sz * (2 << size);

	size = QUP_IO_M_INPUT_FIFO_SIZE(iomode);
	controller->in_fifo_sz = controller->in_blk_sz * (2 << size);

	dev_info(dev, "IN:block:%d, fifo:%d, OUT:block:%d, fifo:%d\n",
		 controller->in_blk_sz, controller->in_fifo_sz,
		 controller->out_blk_sz, controller->out_fifo_sz);

	writel_relaxed(1, base + QUP_SW_RESET);

	ret = spi_qup_set_state(controller, QUP_STATE_RESET);
	if (ret) {
		dev_err(dev, "cannot set RESET state\n");
		goto error;
	}

	writel_relaxed(0, base + QUP_OPERATIONAL);
	writel_relaxed(0, base + QUP_IO_M_MODES);

	if (!controller->qup_v1)
		writel_relaxed(0, base + QUP_OPERATIONAL_MASK);

	writel_relaxed(SPI_ERROR_CLK_UNDER_RUN | SPI_ERROR_CLK_OVER_RUN,
		       base + SPI_ERROR_FLAGS_EN);

	/* allocate dma resources, if available */
	controller->rx_chan = dma_request_slave_channel(&pdev->dev, "rx");
	controller->tx_chan = dma_request_slave_channel(&pdev->dev, "tx");

	if (controller->rx_chan && controller->tx_chan) {


		/* set DMA parameters */
		controller->rx_conf.device_fc = 1;
		controller->rx_conf.src_addr = res->start + QUP_INPUT_FIFO;
		controller->rx_conf.src_maxburst = controller->in_blk_sz;

		controller->tx_conf.device_fc = 1;
		controller->tx_conf.dst_addr = res->start + QUP_OUTPUT_FIFO;
		controller->tx_conf.dst_maxburst = controller->out_blk_sz;

		if (dmaengine_slave_config(controller->rx_chan,
				&controller->rx_conf)) {
			dev_err(&pdev->dev, "failed to configure RX channel\n");

			dma_release_channel(controller->rx_chan);
			dma_release_channel(controller->tx_chan);
			controller->tx_chan = NULL;
			controller->rx_chan = NULL;
		} else if (dmaengine_slave_config(controller->tx_chan,
				&controller->tx_conf)) {
			dev_err(&pdev->dev, "failed to configure TX channel\n");

			dma_release_channel(controller->rx_chan);
			dma_release_channel(controller->tx_chan);
			controller->tx_chan = NULL;
			controller->rx_chan = NULL;
		} else {
			controller->dummy = dma_alloc_writecombine(
				controller->dev,
				controller->in_blk_sz + controller->out_blk_sz,
				&controller->dummy_phys, GFP_KERNEL);

			if (!controller->dummy) {
				dev_err(&pdev->dev,
					"failed to allocate DMA memory\n");

				dma_release_channel(controller->rx_chan);
				dma_release_channel(controller->tx_chan);
				controller->tx_chan = NULL;
				controller->rx_chan = NULL;
			}
		}
	} else {
		if (controller->rx_chan) {
			dma_release_channel(controller->rx_chan);
			controller->rx_chan = NULL;
		} else {
			dev_err(&pdev->dev, "Failed to allocate dma rx chan");
		}
		if (controller->tx_chan) {
			dma_release_channel(controller->tx_chan);
			controller->tx_chan = NULL;
		} else {
			dev_err(&pdev->dev, "Failed to allocate dma tx chan");
		}
	}

	/* if earlier version of the QUP, disable INPUT_OVERRUN */
	if (controller->qup_v1)
		writel_relaxed(QUP_ERROR_OUTPUT_OVER_RUN |
			QUP_ERROR_INPUT_UNDER_RUN | QUP_ERROR_OUTPUT_UNDER_RUN,
			base + QUP_ERROR_FLAGS_EN);

	writel_relaxed(0, base + SPI_CONFIG);
	writel_relaxed(SPI_IO_C_NO_TRI_STATE, base + SPI_IO_CONTROL);

	ret = devm_request_irq(dev, irq, spi_qup_qup_irq,
			       IRQF_TRIGGER_HIGH, pdev->name, controller);
	if (ret)
		goto error;

	pm_runtime_set_autosuspend_delay(dev, MSEC_PER_SEC);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	ret = devm_spi_register_master(dev, master);
	if (ret)
		goto disable_pm;

	return 0;

disable_pm:
	pm_runtime_disable(&pdev->dev);
error:
	clk_disable_unprepare(cclk);
	clk_disable_unprepare(iclk);
	spi_master_put(master);
	return ret;
}

#ifdef CONFIG_PM_RUNTIME
static int spi_qup_pm_suspend_runtime(struct device *device)
{
	struct spi_master *master = dev_get_drvdata(device);
	struct spi_qup *controller = spi_master_get_devdata(master);
	u32 config;

	/* Enable clocks auto gaiting */
	config = readl(controller->base + QUP_CONFIG);
	config |= QUP_CONFIG_CLOCK_AUTO_GATE;
	writel_relaxed(config, controller->base + QUP_CONFIG);
	return 0;
}

static int spi_qup_pm_resume_runtime(struct device *device)
{
	struct spi_master *master = dev_get_drvdata(device);
	struct spi_qup *controller = spi_master_get_devdata(master);
	u32 config;

	/* Disable clocks auto gaiting */
	config = readl_relaxed(controller->base + QUP_CONFIG);
	config &= ~QUP_CONFIG_CLOCK_AUTO_GATE;
	writel_relaxed(config, controller->base + QUP_CONFIG);
	return 0;
}
#endif /* CONFIG_PM_RUNTIME */

#ifdef CONFIG_PM_SLEEP
static int spi_qup_suspend(struct device *device)
{
	struct spi_master *master = dev_get_drvdata(device);
	struct spi_qup *controller = spi_master_get_devdata(master);
	int ret;

	ret = spi_master_suspend(master);
	if (ret)
		return ret;

	ret = spi_qup_set_state(controller, QUP_STATE_RESET);
	if (ret)
		return ret;

	clk_disable_unprepare(controller->cclk);
	clk_disable_unprepare(controller->iclk);
	return 0;
}

static int spi_qup_resume(struct device *device)
{
	struct spi_master *master = dev_get_drvdata(device);
	struct spi_qup *controller = spi_master_get_devdata(master);
	int ret;

	ret = clk_prepare_enable(controller->iclk);
	if (ret)
		return ret;

	ret = clk_prepare_enable(controller->cclk);
	if (ret)
		return ret;

	ret = spi_qup_set_state(controller, QUP_STATE_RESET);
	if (ret)
		return ret;

	return spi_master_resume(master);
}
#endif /* CONFIG_PM_SLEEP */

static int spi_qup_remove(struct platform_device *pdev)
{
	struct spi_master *master = dev_get_drvdata(&pdev->dev);
	struct spi_qup *controller = spi_master_get_devdata(master);
	int ret;

	ret = pm_runtime_get_sync(&pdev->dev);
	if (ret)
		return ret;

	ret = spi_qup_set_state(controller, QUP_STATE_RESET);
	if (ret)
		return ret;

	if (controller->rx_chan)
		dma_release_channel(controller->rx_chan);
	if (controller->tx_chan)
		dma_release_channel(controller->tx_chan);

	clk_disable_unprepare(controller->cclk);
	clk_disable_unprepare(controller->iclk);

	dma_free_writecombine(controller->dev,
			controller->in_blk_sz + controller->out_blk_sz,
			controller->dummy, controller->dummy_phys);

	pm_runtime_put_noidle(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	return 0;
}

static struct of_device_id spi_qup_dt_match[] = {
	{ .compatible = "qcom,spi-qup-v1.1.1", },
	{ .compatible = "qcom,spi-qup-v2.1.1", },
	{ .compatible = "qcom,spi-qup-v2.2.1", },
	{ }
};
MODULE_DEVICE_TABLE(of, spi_qup_dt_match);

static const struct dev_pm_ops spi_qup_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(spi_qup_suspend, spi_qup_resume)
	SET_RUNTIME_PM_OPS(spi_qup_pm_suspend_runtime,
			   spi_qup_pm_resume_runtime,
			   NULL)
};

static struct platform_driver spi_qup_driver = {
	.driver = {
		.name		= "spi_qup",
		.owner		= THIS_MODULE,
		.pm		= &spi_qup_dev_pm_ops,
		.of_match_table = spi_qup_dt_match,
	},
	.probe = spi_qup_probe,
	.remove = spi_qup_remove,
};
module_platform_driver(spi_qup_driver);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:spi_qup");
