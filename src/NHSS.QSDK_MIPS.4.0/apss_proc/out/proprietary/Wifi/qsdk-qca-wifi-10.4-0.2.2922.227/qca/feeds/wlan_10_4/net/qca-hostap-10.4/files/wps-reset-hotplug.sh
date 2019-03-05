#
#
# Copyright (c) 2014, The Linux Foundation. All rights reserved.
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice appear in all copies.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

if [ "$ACTION" = "released" -a "$BUTTON" = "wps" ]; then
	default_hold=3
	if [ -f /var/run/plchost.pid ]
	then
		default_hold=12
	fi
	if [ "$SEEN" -gt $default_hold ]; then
		echo "" > /dev/console
		echo "RESET TO FACTORY SETTING EVENT DETECTED" > /dev/console
		echo "PLEASE WAIT WHILE REBOOTING THE DEVICE..." > /dev/console
		rm -rf /overlay/*
		reboot
	fi
fi
