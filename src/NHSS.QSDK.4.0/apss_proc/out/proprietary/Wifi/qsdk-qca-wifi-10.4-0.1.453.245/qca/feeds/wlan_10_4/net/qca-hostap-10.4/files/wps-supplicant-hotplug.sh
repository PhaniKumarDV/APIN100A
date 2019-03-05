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
#

local pid
if [ "$ACTION" = "pressed" -a "$BUTTON" = "wps" ]; then
	[ -r /var/run/wifi-wps-enhc-extn.conf ] && exit 0
	for dir in /var/run/wpa_supplicant-*; do
		[ -d "$dir" ] || continue
		pid=/var/run/wps-hotplug-${dir#"/var/run/wpa_supplicant-"}.pid
		wpa_cli -p "$dir" wps_pbc
		[ -f $pid ] || {
			wpa_cli -p"$dir" -a/lib/wifi/wps-supplicant-update-uci -P$pid -B
		}
	done
fi
