WLAN_DEVICES=
WLAN_DEVICE_NUM=
ALL_DEVICES=
local guest_network="guest"
local traffic_separation_enabled=0
local traffic_separation_active=0

__whc_get_wlan_vifnum() {
	local config="$1"
	local iface disabled
	local phydev phydisabled

	config_get disabled "$config" disabled '0'
	config_get phydev "$config" device ""

	if [ -z "$phydev" ]; then
		return
	fi

	config_get phydisabled ${phydev} disabled 0
	if [ $phydisabled -eq 0 -a "$disabled" -eq 0 ]; then
		WLAN_DEVICE_NUM=$((WLAN_DEVICE_NUM + 1))
	fi
}

__whc_get_wlan_ifaces() {
	local config="$1"
	local ssid_to_match="$2"
	local network_to_match="$3"
	local device iface disabled mode ssid network

	config_get device "$config" device
	config_get iface "$config" ifname
	config_get disabled "$config" disabled '0'
	config_get mode "$config" mode
	config_get ssid "$config" ssid
	config_get network "$config" network

	if [ "$mode" == "wrap" ]; then
		echo "Interface $iface is in QWrap mode, can not use for lbd"
		return
	fi

	if [ "$traffic_separation_enabled" -gt 0 ] && \
		[ "$traffic_separation_active" -gt 0 ] && \
		[ -n "$network_to_match" -a "$network" != "$network_to_match" ]; then
                return
	fi
         
	if [ -n "$iface" -a "$mode" == "ap" -a "$disabled" -eq 0 ]; then
		if [ ! -n "$ssid_to_match" -o "$ssid" == "$ssid_to_match" ]; then
			WLAN_DEVICES="${WLAN_DEVICES}${WLAN_DEVICES:+","}${device}:${iface}"
		fi
	fi
}

# whc_get_wlan_ifaces()
# output: $1 List of all WLAN interfaces matching the SSID provided
# input: $2 The desired SSID. If it is null string, then get all WLAN
#        interfaces; otherwise, get all that matches this SSID
whc_get_wlan_ifaces() {
	WLAN_DEVICES=""
	config_load 'repacd'
	config_get guest_network repacd NetworkGuest 'guest'
	config_get traffic_separation_enabled repacd TrafficSeparationEnabled '0'
	config_get traffic_separation_active repacd TrafficSeparationActive '0'

	config_load wireless
	config_foreach __whc_get_wlan_ifaces wifi-iface $2

	eval "$1='${WLAN_DEVICES}'"
}
