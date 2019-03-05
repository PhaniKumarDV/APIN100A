#!/bin/sh
# Copyright (c) 2015 Qualcomm Atheros, Inc.
#
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary.

WIFIMON_DEBUG_OUTOUT=0

# Set this to a filename to log all commands executed.
# The output of relevant commands will be appeneded to the file.
WIFIMON_DEBUG_COMMAND_FILE=

WIFIMON_STATE_NOT_ASSOCIATED='NotAssociated'
WIFIMON_STATE_AUTOCONFIG_IN_PROGRESS='AutoConfigInProgress'
WIFIMON_STATE_MEASURING='Measuring'
WIFIMON_STATE_WPS_TIMEOUT='WPSTimeout'
WIFIMON_STATE_BSSID_ASSOC_TIMEOUT='BSSIDAssocTimeout'
WIFIMON_STATE_ASSOC_TIMEOUT='AssocTimeout'
WIFIMON_STATE_RE_MOVE_CLOSER='RE_MoveCloser'
WIFIMON_STATE_RE_MOVE_FARTHER='RE_MoveFarther'
WIFIMON_STATE_RE_LOCATION_SUITABLE='RE_LocationSuitable'
WIFIMON_STATE_CL_LINK_SUFFICIENT='CL_LinkSufficient'
WIFIMON_STATE_CL_LINK_INADEQUATE='CL_LinkInadequate'
WIFIMON_STATE_CL_ACTING_AS_RE='CL_ActingAsRE'

WIFIMON_PIPE_NAME='/var/run/repacd.pipe'

. /lib/functions.sh
. /lib/functions/whc-network.sh

# State information
local sta_iface_24g sta_iface_24g_config_name
local sta_iface_5g sta_iface_5g_config_name unknown_ifaces=0
local disabled_sta_iface_config_name
local otherband_sta_iface otherband_sta_iface_config_name
local measurement_sta_iface measurement_sta_iface_config_name
local assoc_timeout_logged=0 wps_timeout_logged=0
local wps_in_progress=0 wps_start_time=''
local wps_stabilization=0 wps_assoc_count=0
local auto_mode_stabilization=0 auto_mode_assoc_count=0
local bssid_stabilization=0 bssid_assoc_count=0 bssid_resolve_complete=0
local assoc_start_time='' last_assoc_state=0 backhaul_eval_time='' force_down_5g_timestamp=''
local ping_running=0 last_ping_gw_ip
local rssi_num=0 rssi_filename=
local force_down_5g=0 down_time_2g='' measurement_eval_count=0
local force_down_24g=0 badlink_start_time_5g='' badlink_switch_inprogress=0
local rate_num=0 rate_min=0 rate_max=0 rate_pref2G=0 rate_filename=
local rssi_counter=0
local is_24G_down_by_independent_channel=0
local rssi_idle_time=0
local bestap_median bestap_otherband_median bestap_filename='' bestap_otherband_filename=''
local nolink_detected_5g=0

# Config parameters
local device_type config_re_mode default_re_mode
local min_wps_assoc min_auto_mode_assoc min_bssid_assoc
local rssi_samples rssi_far rssi_near rssi_min rssi_pref2G
local assoc_timeout wps_timeout config_downtime2G measuring_attempts=0
local rate_samples percent_rate_min5G percent_rate_max5G percent_rate_pref2G
local daisy_chain bssid_assoc_timeout badlink_timeout5G
local config_short_eval_time5g=0 config_long_eval_time5g=0 config_eval_time24g=0
local traffic_separation_enabled=0 traffic_separation_active=0
local guest_backhaul network_guest
local guest_preflink_down_start_time=0 guest_preflink_down_timeout=15
local guest_preflink_up_start_time=0 guest_preflink_up_timeout=5
local guest_link_override=0 guest_vid=2
local is_independent_channel_selection_enable
local quality_5g_rssi_level
local rssi_check_total_counter

# Emit a message at debug level.
# input: $1 - the message to log
__repacd_wifimon_debug() {
    local stderr=''
    if [ "$WIFIMON_DEBUG_OUTOUT" -gt 0 ]; then
        stderr='-s'
    fi

    logger $stderr -t repacd.wifimon -p user.debug "$1"
}

# Log the output of a command to a file (when enabled).
# This is a nop unless WIFIMON_DEBUG_COMMAND_FILE is set.
# input: $1 - command output to log
__repacd_wifimon_dump_cmd() {
    if [ -n "$WIFIMON_DEBUG_COMMAND_FILE" ]; then
        touch $WIFIMON_DEBUG_COMMAND_FILE
        date >> $WIFIMON_DEBUG_COMMAND_FILE
        echo "$1" >> $WIFIMON_DEBUG_COMMAND_FILE
        echo >> $WIFIMON_DEBUG_COMMAND_FILE
    fi
}

# Emit a message at info level.
__repacd_wifimon_info() {
    local stderr=''
    if [ "$WIFIMON_DEBUG_OUTOUT" -gt 0 ]; then
        stderr='-s'
    fi

    logger $stderr -t repacd.wifimon -p user.info "$1"
}

# Obtain a timestamp from the system.
#
# These timestamps will be monontonically increasing and be unaffected by
# any time skew (eg. via NTP or manual date commands).
#
# output: $1 - the timestamp as an integer (with any fractional time truncated)
__repacd_wifimon_get_timestamp() {
    timestamp=`cat /proc/uptime | cut -d' ' -f1 | cut -d. -f 1`
    eval "$1=$timestamp"
}

# Terminate any background ping that may be running.
# If no background pings are running, this will be a nop.
__repacd_stop_ping() {
    if [ "$ping_running" -gt 0 ]; then
        kill $(jobs -p)
        ping_running=0
        __repacd_wifimon_debug "Stopped ping to GW IP $last_ping_gw_ip"
    fi

    if [ -n "$rssi_filename" ]; then
        # Clean up the temporary file
        rm -f $rssi_filename
        rssi_filename=
    fi
    if [ -n "$rate_filename" ]; then
        # Clean up the temporary file
        rm -f $rate_filename
        rate_filename=
    fi
    if [ -n "$bestap_filename" ]; then
        # Clean up the temporary file
        rm -f $bestap_filename
        bestap_filename=
    fi
    if [ -n "$bestap_otherband_filename" ]; then
        # Clean up the temporary file
        rm -f $bestap_otherband_filename
        bestap_otherband_filename=
    fi
}

# Start a background ping to the gateway address (if it can be resolved).
# This helps ensure the RSSI values are updated (as firmware will not report
# updates if only beacons are being received on the STA interface).
# input: $1 - network: the name of the network being managed
# return: 0 if the ping was started or is already running; otherwise 1
__repacd_start_ping() {
    gw_ip=`route -n | grep ^0.0.0.0 | grep br-$1 | awk '{print $2}'`
    if [ -n "$gw_ip" ]; then
        if [ ! "$gw_ip" = "$last_ping_gw_ip" ]; then
            # First need to kill the existing one due to the IP change.
            __repacd_stop_ping
            # This will leave ping_running set to 0.
        fi

        if [ "$ping_running" -eq 0 ]; then
            __repacd_wifimon_debug "Pinging GW IP $gw_ip"

            # Unfortunately the busybox ping command does not support an
            # interval. Thus, we can only ping once per second so there will
            # only be a handful of measurements over the course of our RSSI
            # sampling.
            ping $gw_ip > /dev/null &
            ping_running=1
            last_ping_gw_ip=$gw_ip
        fi

        # Ping is running now or was started.
        return 0
    fi

    __repacd_wifimon_info "Failed to resolve GW when starting ping; will re-attempt"
    return 1
}

# Determine if the gateway is reachable.
#
# Ideally this would be limited to only the 5 GHz STA interface, but there
# is no good way to do this (since packets would need to be received on the
# bridge interface).
#
# return: 0 if the gateway is reachable; otherwise 1
__repacd_is_gw_reachable() {
    if [ -n "$last_ping_gw_ip" ]; then
        if ping -c 1 -W 1 ${last_ping_gw_ip} > /dev/null; then
            return 0
        fi
    fi

    # Gateway is unknown or is not reachable
    return 1
}

# Determine if the STA interface named is current associated and active.
#
# input: $1 - sta_iface: the name of the interface (eg. ath01)
# return: 0 if associated; 1 if not associated or empty interface name
__repacd_wifimon_is_active_assoc() {
    local sta_iface=$1
    if [ -n "$sta_iface" ]; then
        local assoc_str=$(iwconfig $sta_iface)
        __repacd_wifimon_dump_cmd "State of $sta_iface: $assoc_str"

        if $(echo "$assoc_str" | grep 'Access Point: ' | grep -v 'Not-Associated' > /dev/null); then
            return 0
        else
            return 1
        fi
    else
        # An unknown STA interface is considered not associated.
        return 1
    fi
}

# Determine if the STA interface named is current associated.
#
# Note that for the purposes of this function, an empty interface name is
# considered associated. This is done because in some configurations, only
# one interface is enabled.
#
# input: $1 - sta_iface: the name of the interface (eg. ath01)
# return: 0 if associated or if the interface name is empty; otherwise 1
__repacd_wifimon_is_assoc() {
    local sta_iface=$1
    if [ -n "$sta_iface" ];
    then
        if [ "$sta_iface" = "$sta_iface_5g" ] &&
               [ "$force_down_5g" -gt 0 ];then
            return 0
        elif [ "$sta_iface" = "$sta_iface_24g" ] \
            && [ "$force_down_24g" -gt 0 ]; then
            return 0
        elif [ "$sta_iface" = "$sta_iface_24g" ] \
            && [ "$is_24G_down_by_independent_channel" -gt 0 ]; then
            return 0
        fi

        if __repacd_wifimon_is_active_assoc $sta_iface; then
            return 0
        else
            return 1
        fi
    else
        # An unknown STA interface is considered associated.
        return 0
    fi
}

# Determine if the device is already operating in the desired range extender
# mode when using automatic mode switching.
#
# input: $1 - cur_re_mode: the current operating range extender mode
# input: $2 - cur_re_submode: the current operating range extender sub-mode
# output: $3 - new_re_mode: the new range extender mode
# output: $4 - new_re_submode: the new range extender sub-mode
# return: 0 if already operating in the desired mode & submode; otherwise 1
__repacd_wifimon_resolve_mode() {
    local old_re_mode=$1
    local old_re_submode=$2
    local re_mode_changed=0

    # Finally, if the serving AP is operating in one of the special
    # modes, write back the association derived RE mode. Otherwise,
    # write back the default mode.
    if [ "$config_re_mode" = 'auto' ]; then
        # Since when operating in SON mode we rely on wsplcd/daisy to force
        # the association to the CAP, do not check whether the CAP is
        # the serving AP when determining whether to switch modes.
        if __repacd_wifimon_is_serving_ap_son; then
            if [ ! "$old_re_mode" = 'son' ]; then
                __repacd_wifimon_info "Serving AP has SON enabled"
                eval "$3=son"
                re_mode_changed=1
            fi
        elif __repacd_wifimon_is_serving_ap_wds; then
            if [ ! "$old_re_mode" = 'wds' ]; then
                __repacd_wifimon_info "Serving AP has WDS enabled"
                eval "$3=wds"
                re_mode_changed=1
            fi
        else
            if [ ! "$old_re_mode" = "$default_re_mode" ]; then
                __repacd_wifimon_info "Serving AP does not advertise WDS"
                eval "$3=$default_re_mode"
                re_mode_changed=1
            fi
        fi
    fi

    # By definition, when operating in non-auto mode, we are always in
    # the desired mode. But if there is any special sub-modes, handle it here.
    if __repacd_wifimon_is_serving_ap_son; then
        __repacd_wifimon_get_son_submode submode
        if [ ! "$old_re_submode" = "$submode" ]; then
            eval "$4=$submode"
            re_mode_changed=1
        fi
    fi

    if [ "$re_mode_changed" -gt 0 ]; then
        return 1
    else
        return 0
    fi
}

# Determine if the peer BSSID has been written back to UCI.
# This assumes the wireless config file has already been loaded.
# input: $1 - iface_section: the name of the section in UCI
# return: 0 if the BSSID has been set or there is no section; otherwise 1
__repacd_wifimon_is_peer_bssid_set() {
    local iface_section=$1

    if [ -z "$iface_section" ]; then
        # Nothing to clone.
        return 0
    elif [ -n "$iface_section" ]; then
        config_get peer_bssid $iface_section 'bssid' ''
        if [ -n "$peer_bssid" ]; then
            return 0
        fi
    fi

    return 1
}

# Determine if the deep cloning process has completed (if enabled).
# return: 0 if the process is complete or is not enabled; otherwise 1
__repacd_wifimon_is_deep_cloning_complete() {
    # First check if wsplcd and deep cloning are even enabled. If not,
    # then consider it complete.
    local wsplcd_enabled
    local deep_cloning_enabled
    local config_sta_enabled
    local peer_bssid

    config_load wsplcd
    config_get wsplcd_enabled 'config' 'HyFiSecurity' '0'
    config_get deep_cloning_enabled 'config' 'DeepClone' '0'
    config_get config_sta_enabled 'config' 'ConfigSta' '0'

    if [ "$wsplcd_enabled" -eq 0 -o "$deep_cloning_enabled" -eq 0 -o \
         "$config_sta_enabled" -eq 0 ]; then
        return 0
    fi

    # For each of the STA interfaces, see if the peer BSSID has been set.
    config_load wireless
    __repacd_wifimon_is_peer_bssid_set $sta_iface_24g_config_name || return 1
    __repacd_wifimon_is_peer_bssid_set $sta_iface_5g_config_name || return 1

    return 0
}

# Determine if the STA association is stable enough to be able to start
# the next step of the process.
# return: 0 if the association is stable; non-zero if it is not yet deemed
#         stable
__repacd_wifimon_is_assoc_stable() {
    if [ "$bssid_stabilization" -gt 0 ]; then
        if [ "$bssid_assoc_count" -ge "$min_bssid_assoc" ]; then
            return 0
        else
            return 1
        fi
    elif [ "$wps_stabilization" -gt 0 ]; then
        if [ "$wps_assoc_count" -ge "$min_wps_assoc" ]; then
            return 0
        else
            return 1
        fi
    elif [ "$auto_mode_stabilization" -gt 0 ]; then
        if [ "$auto_mode_assoc_count" -ge "$min_auto_mode_assoc" ]; then
            return 0
        else
            return 1
        fi
    else
        # No stabilization in progress
        return 0
    fi
}

# Determine if the guest network's backhaul interface should be changed
# from prefered interface to non-prefered interface based on link status.
# If change has already made then monitor prefered interface, if prefered
# interface link looks good then change back to prefered interface.
# input: $1 - pref_iface: prefered sta interface
# input: $2 - other_iface: other non prefered interface
__repacd_wifimon_override_guest_backhaul() {
    local pref_iface=$1
    local other_iface=$2
    local force_pref_down=0
    local force_other_down=0
    local ifname

    if [ "$pref_iface" = "$sta_iface_5g" ]; then
        force_pref_down=$force_down_5g
        force_other_down=$force_down_24g
    elif [ "$pref_iface" = "$sta_iface_24g" ]; then
        force_pref_down=$force_down_24g
        force_other_down=$force_down_5g
    fi

    if ! __repacd_wifimon_is_assoc $pref_iface || \
           [ "$force_pref_down" -gt 0 ]; then
        if __repacd_wifimon_is_assoc $other_iface &&
               [ "$force_other_down" -eq 0 ]; then
            if [ "$guest_link_override" -eq 0 ]; then
                if [ "$guest_preflink_down_start_time" -eq 0 ]; then
                    __repacd_wifimon_get_timestamp guest_preflink_down_start_time
                    guest_preflink_up_start_time=0
                elif __repacd_wifimon_is_timeout $guest_preflink_down_start_time \
                         $guest_preflink_down_timeout; then
                    ifname=`iwconfig 2>&1 | grep "$other_iface.$guest_vid" | cut -d ' ' -f1`
                    if [ -z "$ifname" ]; then
                        vconfig add $other_iface $guest_vid
                    fi
                    brctl delif "br-$network_guest" "$pref_iface.$guest_vid"
                    brctl addif "br-$network_guest" "$other_iface.$guest_vid"
                    ifconfig $other_iface.$guest_vid up
                    guest_link_override=1
                fi
            fi
        elif ! __repacd_wifimon_is_assoc $other_iface || \
                 [ "$force_other_down" -gt 0 ]; then
            if [ -n "$guest_preflink_down_start_time" ]; then
                guest_preflink_down_start_time=0
            fi

            if [ "$guest_link_override" -gt 0 ]; then
                brctl delif "br-$network_guest" "$other_iface.$guest_vid"
                brctl addif "br-$network_guest" "$pref_iface.$guest_vid"
                ifconfig $pref_iface.$guest_vid up
                guest_link_override=0
            fi
        fi
    elif __repacd_wifimon_is_assoc $pref_iface &&
             [ "$force_pref_down" -eq 0 ]; then
        if [ "$guest_link_override" -gt 0 ]; then
           if [ "$guest_preflink_up_start_time" -eq 0 ]; then
               __repacd_wifimon_get_timestamp guest_preflink_up_start_time
               guest_preflink_down_start_time=0
           elif __repacd_wifimon_is_timeout $guest_preflink_up_start_time \
                    $guest_preflink_up_timeout; then
               brctl delif "br-$network_guest" "$other_iface.$guest_vid"
               brctl addif "br-$network_guest" "$pref_iface.$guest_vid"
               ifconfig $pref_iface.$guest_vid up
               guest_link_override=0
           fi
        fi
    fi
}

# Determine the 5G RSSI backhaul quality
# input $1 -- 5G RSSI value
__repacd_wifimon_independent_channel_check() {
    local rssi="$1"
    if [ "$rssi_idle_time" -eq 0 ]; then
        if [ -n "$sta_iface_24g" ]; then
            __repacd_wifimon_debug "rssi = $rssi quality_5g_rssi_level=$quality_5g_rssi_level force_down_24g=$force_down_24g force_down_5g=$force_down_5g rssi_counter=$rssi_counter"
            if [ "$rssi" -gt -95 -a "$rssi" -lt 0 ]; then
                if [ "$rssi" -gt "$quality_5g_rssi_level" ]; then
                    __repacd_wifimon_debug "rssi good then quality_5g_rssi_level"
                    if [ "$is_24G_down_by_independent_channel" -eq 0 ]; then
                        rssi_counter=$((rssi_counter + 1))
                    else
                        rssi_counter=0
                    fi
                else
                    __repacd_wifimon_debug "rssi less then quality_5g_rssi_level force_down_24g=$force_down_24g"
                    if [ "$is_24G_down_by_independent_channel" -eq 0 ]; then
                        rssi_counter=0
                    else
                        rssi_counter=$((rssi_counter + 1))
                    fi
                fi
            fi
        else
            __repacd_wifimon_debug "sta_iface_24g=$sta_iface_24g"
        fi
    else
        rssi_idle_time=$((rssi_idle_time - 1))
        __repacd_wifimon_debug "rssi_idle_time=$rssi_idle_time"
    fi
}

# Determine if the STA is associated and update the state accordingly.
# input: $1 - network: the name of the network being managed
# input: $2 - cur_re_mode: the currently configured range extender mode
# input: $3 - cur_re_submode: the currently configured range extender sub-mode
# input: $4 - whether this is a check during init for a restart triggered
#             by mode switching
# output: $5 - state: the variable to update with the new state name (if there
#                     was a change)
# output: $6 - re_mode: the desired range extender mode
# output: $7 - re_submode: the desired range extender sub-mode
# return: 0 if associated; otherwise 1
__repacd_wifimon_check_associated() {
    local network=$1
    local cur_re_mode=$2
    local cur_re_submode=$3
    local autoconf_start=$4

    # In Daisy chaining, when we are at hop 2 and above, allow monitoring
    # even if single backhaul is associated provided other backhaul is forced down.
    if __repacd_wifimon_is_assoc $sta_iface_24g \
        && __repacd_wifimon_is_assoc $sta_iface_5g; then
        # Only update the LED state if we transitioned from not associated
        # to associated.
        if [ "$last_assoc_state" -eq 0 ]; then
            if [ "$wps_in_progress" -gt 0 ]; then
                # If WPS was triggered, it could take some time for the
                # interfaces to settle into their final state. Thus, update
                # the start time for the measurement to the point at which
                # the WPS button was pressed.
                assoc_start_time=$wps_start_time

                # Clear this as we only want to extend the association time
                # for this one instance. All subsequent ones should be based
                # on the time we detect a disassociation (unless WPS is
                # triggered again).
                wps_start_time=''

                # Clear this flag so that we now use the association timer
                # instead of the WPS one.
                wps_in_progress=0

                wps_stabilization=1
            elif [ "$daisy_chain" -gt 0 -a "$bssid_stabilization" -eq 0 ]; then
                if [ -n "$sta_iface_5g" -a "$force_down_5g" -eq 0 ]; then
                    # Daisy chaining enabled, so deep cloning would have been disabled.
                    # Update the 5G BSSID, because we will proceed assuming current
                    # connection is stable.
                    if [ "$badlink_switch_inprogress" -eq 0 ]; then
                        __repacd_wifimon_config_current_bssid $sta_iface_5g $sta_iface_5g_config_name
                        bssid_resolve_complete=0
                    else
                        bssid_resolve_complete=1
                    fi

                    __repacd_wifimon_get_timestamp assoc_start_time

                    bssid_stabilization=1
                    bssid_assoc_count=0
                    assoc_timeout_logged=0

                    eval "$5=$WIFIMON_STATE_AUTOCONFIG_IN_PROGRESS"
                elif [ -n "$sta_iface_24g" -a "$force_down_24g" -eq 0 ]; then
                    # We are here because we detected Nolink or Badlink in 5G band.
                    __repacd_wifimon_config_current_bssid $sta_iface_24g $sta_iface_24g_config_name
                    bssid_resolve_complete=0

                    __repacd_wifimon_get_timestamp assoc_start_time

                    bssid_stabilization=1
                    bssid_assoc_count=0
                    assoc_timeout_logged=0

                    eval "$5=$WIFIMON_STATE_AUTOCONFIG_IN_PROGRESS"
                fi
            elif [ "$config_re_mode" = 'auto' ] && [ "$auto_mode_stabilization" -eq 0 ]; then
                # When making mode switching decisions, we also want to allow
                # for enough time for the interfaces to stabilize.
                __repacd_wifimon_get_timestamp assoc_start_time

                auto_mode_stabilization=1
                auto_mode_assoc_count=0
                assoc_timeout_logged=0

                eval "$5=$WIFIMON_STATE_AUTOCONFIG_IN_PROGRESS"
            fi

            if [ "$wps_stabilization" -gt 0 ]; then
                wps_assoc_count=$((wps_assoc_count + 1))
                __repacd_wifimon_debug "Assoc post WPS (#$wps_assoc_count)"
            elif [ "$bssid_stabilization" -gt 0 ]; then
                if [ "$bssid_resolve_complete" -gt 0 ]; then
                    bssid_assoc_count=$((bssid_assoc_count + 1))
                    __repacd_wifimon_debug "Assoc post BSSID resolve (#$bssid_assoc_count)"
                elif [ "$cur_re_mode" = "son" -a -n "$disabled_sta_iface_config_name" ]; then
                    # Try configuring otherband backhaul BSSID.
                    # Otherband BSSID is arrived from beacon of current associated link.
                    # If successfully configured, we are ready to enable
                    # otherband station and check for association.
                    if [ -z "$sta_iface_24g" ]; then
                        __repacd_wifimon_find_and_config_otherband_bssid $sta_iface_5g
                    else
                        __repacd_wifimon_find_and_config_otherband_bssid $sta_iface_24g
                    fi

                    config_load wireless
                    if __repacd_wifimon_is_peer_bssid_set $disabled_sta_iface_config_name; then
                        # Marking BSSIDResolveState=new to restart the RE.
                        # Restarting is required since we need to enable 2.4G backhaul.
                        uci_set repacd WiFiLink BSSIDResolveState 'new'
                        uci_commit repacd
                        bssid_resolve_complete=1
                    else
                        __repacd_wifimon_debug "Waiting for BSSID resolve"
                    fi
                else
                    # All backhaul stations are already up,
                    # Marking BSSIDResolveState=resolved to indicate BSSID resolve complete.
                    uci_set repacd WiFiLink BSSIDResolveState 'resolved'
                    uci_commit repacd
                    bssid_resolve_complete=1
                fi
            elif [ "$auto_mode_stabilization" -gt 0 ]; then
                if __repacd_wifimon_is_deep_cloning_complete; then
                    auto_mode_assoc_count=$((auto_mode_assoc_count + 1))
                    __repacd_wifimon_debug "Auto mode stabilization (#$auto_mode_assoc_count)"
                else
                    __repacd_wifimon_debug "Waiting for deep cloning"
                fi
            fi

            if __repacd_wifimon_is_assoc_stable; then
                auto_mode_stabilization=0
                bssid_stabilization=0

                # Check the mode to see if we are already in the desired mode.
                # If not, we will want to trigger the mode switch first as
                # otherwise the RSSI measurements may not be updated (due to
                # pings not going through).
                if __repacd_wifimon_resolve_mode $cur_re_mode $cur_re_submode $6 $7; then
                    eval "$5=$WIFIMON_STATE_MEASURING"
                    assoc_start_time=''
                    last_assoc_state=1
                    wps_stabilization=0

                    # If daisy chain mode is enabled, get the phyrate of the associated sta interface
                    # and calculate the min and max threshold rates from the configured percent_rate_min5G
                    # and percent_rate_max5G value.
                    if [ "$daisy_chain" -gt 0 ]; then
                        __repacd_wifimon_set_rate_measurement_iface "5g"
                    fi

                    # Restart the measurements. We do not remember any past
                    # ones as they might not reflect the current state (eg.
                    # if the root AP was moved).
                    rssi_num=0
                    rate_num=0
                else
                    # RE mode switch is required. Do not start measuring link.
                    return 0
                fi
            else
                # Pretend like we are not associated since we need it to be
                # stable.
                return 1
            fi
        fi

        # Association is considered stable. Measure the link, we measure the WiFi link
        # based on RSSI or Rate, RSSI measurement is used in star topology where we
        # know the APs location and REs placement is adjusted based on LED feedback.
        # But in Daisy chain, REs might have connected to any best link, REs auto detects
        # the best link so we will not know the position of connected AP and LED
        # feedback is not useful for the user. First make sure all sta interfaces are under
        # our control.
        if [ "$force_down_24g" -gt 0 ] \
            && __repacd_wifimon_is_active_assoc $sta_iface_24g; then
            __repacd_wifimon_bring_iface_down $sta_iface_24g
        fi

        if [ "$force_down_5g" -gt 0 ] \
            && __repacd_wifimon_is_active_assoc $sta_iface_5g; then
            __repacd_wifimon_bring_iface_down $sta_iface_5g
        fi

        # Make sure all the interfaces are broadcasting right hop count.
        if [ "$force_down_24g" -gt 0 -o "$force_down_5g" -gt 0 ]; then
            config_load wireless
            config_foreach __repacd_wifimon_set_root_distance wifi-iface
        fi

        # Waiting for 2.4G backhaul to associate in case of badlink.
        # Bring down 5G backhaul once 2.4G is associated and stable.
        if [ "$badlink_switch_inprogress" -gt 0 ] \
            && __repacd_wifimon_is_assoc $sta_iface_24g; then
            __repacd_wifimon_bring_iface_down $sta_iface_5g
            badlink_switch_inprogress=0
            badlink_start_time_5g=''
        fi

        if [ "$daisy_chain" -gt 0 -a "$force_down_5g" -eq 0 ]; then
            # Measure the link rate, so that we trigger the best link selection logic.
            if [ "$rate_num" -le "$rate_samples" ] && \
                __repacd_start_ping $network; then
                __repacd_wifimon_measure_rate $network $5
                return 0
            else
                # Restart the rate measurement
                rate_num=0
                #make sure it can continue to check the RSSI
                if [ "$is_independent_channel_selection_enable" -gt 0 ] &&\
                __repacd_start_ping $network; then
                    __repacd_wifimon_independent_channel_check`iwconfig $sta_iface_5g | grep 'Signal level' | awk -F'=' '{print $3}' | awk '{print $1}'`
                fi
            fi

        else
            if [ "$force_down_5g" -gt 0 ]; then
                eval "$5=$WIFIMON_STATE_RE_MOVE_CLOSER"
                return 0;
            fi

            # Association is considered stable. Measure the link RSSI.
            if [ "$rssi_num" -le "$rssi_samples" ] && \
                __repacd_start_ping $network; then
                __repacd_wifimon_measure_link $network $5
            else
                #make sure it can continue to check the RSSI
                if [ "$is_independent_channel_selection_enable" -gt 0 ] &&\
                      __repacd_start_ping $network; then
                    __repacd_wifimon_independent_channel_check `iwconfig $sta_iface_5g | grep 'Signal level' | awk -F'=' '{print $3}' | awk '{print $1}'`
                fi
            fi
        fi
        return 0
    # All cases below are for not associated.
    elif [ "$autoconf_start" -gt 0 ]; then
        # When making mode switching decisions, we also want to allow
        # for enough time for the interfaces to stabilize.
        __repacd_wifimon_get_timestamp assoc_start_time

        auto_mode_stabilization=1
        auto_mode_assoc_count=0
        assoc_timeout_logged=0

        eval "$5=$WIFIMON_STATE_AUTOCONFIG_IN_PROGRESS"
    elif [ "$wps_in_progress" -eq 0 -a "$wps_stabilization" -eq 0 -a \
           "$auto_mode_stabilization" -eq 0 -a "$bssid_stabilization" -eq 0 ]; then
        # Record the first time we detected ourselves as not being associated.
        # This will drive a timer in the check function that will change the
        # state if we stay disassociated for too long.
        if [ -z "$assoc_start_time" ]; then
            __repacd_wifimon_get_timestamp assoc_start_time
        fi
        if [ "$is_independent_channel_selection_enable" -gt 0 ];then
            if  __repacd_wifimon_is_assoc $sta_iface_5g && \\
            __repacd_start_ping $network; then
            __repacd_wifimon_independent_channel_check `iwconfig $sta_iface_5g | grep 'Signal level' | awk -F'=' '{print $3}' | awk '{print $1}'`
            fi
        else
            __repacd_stop_ping
        fi
        last_assoc_state=0
        eval "$5=$WIFIMON_STATE_NOT_ASSOCIATED"
    elif [ "$wps_in_progress" -gt 0 -o "$auto_mode_stabilization" -gt 0 \
        -o "$bssid_stabilization" -gt 0 ]; then
        if [ "$wps_timeout_logged" -eq 0 -a "$assoc_timeout_logged" -eq 0 ]; then
            # Suppress logs after we've timed out
            __repacd_wifimon_debug "Auto config in progress - not assoc"
        fi

        bssid_assoc_count=0
        wps_assoc_count=0
        auto_mode_assoc_count=0
    fi

    # Not associated and WPS is in progress. No LED update.
    return 1
}

# Check whether the given WHC feature is advertised by the AP on which the
# given STA interface is connected.
# input: $1 - sta_iface: the interface to check for the AP capabilities
# input: $2 - ioctl_name: the name of the private ioctl to use to check the
#                         feature
# return: 0 if the feature is on or the interface name is invalid; otherwise 1
__repacd_wifimon_is_whc_feature_on_iface() {
    local sta_iface=$1
    local ioctl_name=$2
    local command_result

    if [ -z "$sta_iface" ]; then
        return 0
    fi

    if [ "$sta_iface" = "$sta_iface_5g" ] &&
           [ "$force_down_5g" -gt 0 ]; then
        return 0
    elif [ "$sta_iface" = "$sta_iface_24g" ] &&
           [ "$force_down_24g" -gt 0 ]; then
        return 0
    fi

    command_result=$(iwpriv $sta_iface $ioctl_name)
    __repacd_wifimon_dump_cmd "$ioctl_name on $sta_iface: $command_result"

    if [ -n "$command_result" ]; then
        local feature_enabled
        feature_enabled=$(echo "$command_result" | cut -d: -f2)
        if [ "$feature_enabled" -gt 0 ]; then
            return 0
        fi
    else
        __repacd_wifimon_debug "iwpriv failed on $sta_iface for $ioctl_name"
    fi

    # Feature must not be enabled or we cannot resolve it.
    return 1
}

# Determine if the serving AP has WDS enabled or not on all valid interfaces.
# return: 0 if it does have WDS enabled; otherwise 1
__repacd_wifimon_is_serving_ap_wds() {
    if __repacd_wifimon_is_whc_feature_on_iface "$sta_iface_24g" 'get_whc_wds' &&
       __repacd_wifimon_is_whc_feature_on_iface "$sta_iface_5g" 'get_whc_wds'; then
        return 0
    fi

    return 1
}

# Determine if the serving AP has SON mode enabled or not.
# return: 0 if it does have SON enabled; otherwise 1
__repacd_wifimon_is_serving_ap_son() {
    if __repacd_wifimon_is_whc_feature_on_iface "$sta_iface_24g" 'get_whc_son' &&
       __repacd_wifimon_is_whc_feature_on_iface "$sta_iface_5g" 'get_whc_son'; then
        return 0
    fi

    return 1
}

# Determine if the serving AP has SON mode enabled and it is an RE.
# return: 0 if it is RE and does have SON enabled; otherwise 1
__repacd_wifimon_is_serving_ap_daisy_son() {
    if [ "$daisy_chain" -gt 0 ] \
        && __repacd_wifimon_is_whc_feature_on_iface "$sta_iface_5g" 'get_whc_son'; then
        __repacd_get_root_ap_dist $sta_iface_5g root_distance
        if [ "$root_distance" -gt 1 ]; then
            return 0
        fi
    fi

    return 1
}

# Determine the number of hops a given STA interface is from the
# root AP.
# input: $1 - sta_iface: the name of the STA interface (eg. ath01)
# output: $2 - dist: the distance (in terms of hops) from the root AP
__repacd_get_root_ap_dist() {
    local sta_iface=$1

    local command_result

    if [ -z "$sta_iface" ]; then
        return 0
    fi

    command_result=$(iwpriv $sta_iface get_whc_dist)
    __repacd_wifimon_dump_cmd "root dist for $sta_iface: $command_result"

    if [ -n "$command_result" ]; then
        local root_dist
        root_dist=$(echo "$command_result" | cut -d: -f2)
        eval "$2=$root_dist"
    fi
}

# Determine the maximum number of hops the STA interfaces are from the
# root AP.
# output: $1 - max_dist: the maximum distance (in terms of hops) from the
#                       root AP
__repacd_get_max_root_ap_dist() {
    local root_dist_24g=0 root_dist_5g=0
    if [ -n "$sta_iface_24g" ]; then
        __repacd_get_root_ap_dist $sta_iface_24g root_dist_24g
    fi

    if [ -n "$sta_iface_5g" ]; then
        __repacd_get_root_ap_dist $sta_iface_5g root_dist_5g
    fi

    # If daisy chaining enabled and 5G is active,
    # determine root distance from 5G only.
    if [ "$daisy_chain" -gt 0 ] \
        && [ -n "$sta_iface_5g" -a "$force_down_5g" -eq 0 ]; then
            eval "$1=$root_dist_5g"
    else
        if [ "$root_dist_24g" -gt "$root_dist_5g" ]; then
            eval "$1=$root_dist_24g"
        else
            eval "$1=$root_dist_5g"
        fi
    fi
}

# Determine if the serving AP is currently the CAP or not.
# return: 0 if associated to the CAP; otherwise 1
__repacd_wifimon_is_cap_serving() {
    # If the root AP is not operating in WDS mode, it may not even be a WHC
    # AP. To be conservative, we do not enable the AP functionality.
    local wds_enabled root_dist

    if __repacd_wifimon_is_serving_ap_wds; then
        __repacd_get_max_root_ap_dist root_dist
        if [ "$root_dist" -eq 1 ]; then
            # The STA device saying it is 1 hop away means that the serving
            # AP indicated it was 0 hops away from the CAP. This means that
            # the serving AP is the CAP.
            return 0
        fi
    else
        __repacd_wifimon_debug "Serving AP is not WHC enabled"
    fi

    # Either we are more than 1 hop from the root or the distance is
    # unknown. In either case, indicate that the CAP is not serving.
    return 1
}

__repacd_wifimon_set_root_distance() {
    local config=$1
    local root_distance_24g root_distance_5g
    local iface backhaul_ap

    config_get iface $config ifname
    config_get backhaul_ap $config backhaul_ap '0'
    if [ "$backhaul_ap" -gt 0 ]; then
        __repacd_get_root_ap_dist $sta_iface_24g root_distance_24g
        __repacd_get_root_ap_dist $sta_iface_5g root_distance_5g
        if [ "$root_distance_24g" != "$root_distance_5g" ]; then
            if [ "$force_down_5g" -gt 0 ] && whc_is_5g_vap $config; then
                iwpriv $iface set_whc_dist $root_distance_24g
                __repacd_wifimon_debug "$iface: Root distance set to $root_distance_24g"
            elif [ "$force_down_24g" -gt 0 ] && ! whc_is_5g_vap $config; then
                iwpriv $iface set_whc_dist $root_distance_5g
                __repacd_wifimon_debug "$iface: Root distance set to $root_distance_5g"
            fi
        fi
    fi
}

# Determine the current SON sub-mode.
# output: $1 - submode: sub mode we are operating in.
__repacd_wifimon_get_son_submode() {
    if __repacd_wifimon_is_cap_serving; then
        eval "$1=star"
    else
        eval "$1=daisy"
    fi
}

# Bring down sta vap interface.
# input: $1 - sta interface: the name of the interface for bringing down.
__repacd_wifimon_bring_iface_down() {

    local sta_iface=$1
    if [ -n "$sta_iface" ];then
        wpa_cli -p /var/run/wpa_supplicant-$sta_iface disable_network 0
        __repacd_wifimon_info "Interface $sta_iface Brought down "
        if [ "$sta_iface" = "$sta_iface_5g" ]; then
            force_down_5g=1
            if [ -n "$force_down_5g_timestamp" ] ;then
                backhaul_eval_time=$config_long_eval_time5g
            else
                backhaul_eval_time=$config_short_eval_time5g
            fi
            __repacd_wifimon_get_timestamp force_down_5g_timestamp
        else
            force_down_24g=1
        fi
        rssi_counter=0
    fi
}

# Bring up sta vap interface.
# input: $1 - sta interface: the name of the interface for bringing up.
__repacd_wifimon_bring_iface_up() {
    local sta_iface=$1

    if [ -n "$sta_iface" ];then
        wpa_cli -p /var/run/wpa_supplicant-$sta_iface enable_network 0
        __repacd_wifimon_info "Interface $sta_iface Brought up "
        if [ "$sta_iface" = "$sta_iface_5g" ]; then
            force_down_5g=0
        else
            force_down_24g=0
            #if 2.4G interface up, force reset independent_channel parameters to 0
            is_24G_down_by_independent_channel=0
        fi
        rssi_counter=0
        last_assoc_state=0
    fi
}

# Find the median from the samples file.
# Find the median from the samples file.
# This is a crude way to compute the median when the number of
# samples is odd. It is not strictly correct for an even number
# of samples since it does not compute the average of the two
# samples in the middle and rather just takes the lower one, but
# this should be sufficient for our purposes. The average is not
# performed due to the values being on the logarithmic scale and
# because shell scripts do not directly support floating point
# arithmetic.
# input: $1 - Samples filename
# input: $2 - Number of samples in the samples file
# output: $3 - computed Median
__repacd_wifimon_compute_median() {
    local median median_index
    local samples_filename="$1"

    median_index=$((($2 + 1) / 2))
    median=$(cat $samples_filename | sort -n |
                        head -n $median_index | tail -n 1)

    eval "$3=$median"
}

# Measure the Rate to the serving AP and update the state accordingly.
# input: $1 - network: the name of the network being monitored
# output: $2 - state: the variable to update with the new state name (if there
#                     was a change)
__repacd_wifimon_measure_rate() {
    local rate bestap bestap_otherband
    local rssi

    # Just check Gateway link and proceed to rate measurement.
    if ! __repacd_is_gw_reachable; then
        if [ -n "$last_ping_gw_ip" ]; then
            __repacd_wifimon_debug "GW ${last_ping_gw_ip} not reachable"
        else
            __repacd_wifimon_debug "GW unknown"
        fi
    fi

    # Only the 5 GHz link is measured. This is especially done since we
    # generally cannot control which interface is used to reach upstream.
    # Generally 5 GHz will be used (per the rules to set the broadcast bit
    # and choose a default path), so we may not have any valid Rate data on
    # 2.4 GHz.
    rate=`iwpriv $measurement_sta_iface get_whc_rate | awk -F':' '{print $2}'`

    # Get the current Best AP, later it will be used for finding the Best AP
    # once the measurement is completed.
    __repacd_wifimon_get_current_best_ap $measurement_sta_iface bestap bestap_otherband

    if [ "$is_independent_channel_selection_enable" -gt 0 ]; then
        # Only the 5 GHz link is measured. This is especially done since we
        # generally cannot control which interface is used to reach upstream.
        # Generally 5 GHz will be used (per the rules to set the broadcast bit
        # and choose a default path), so we may not have any valid RSSI data on
        # 2.4 GHz.
        rssi=`iwconfig $sta_iface_5g | grep 'Signal level' | awk -F'=' '{print $3}' | awk '{print $1}'`
        __repacd_wifimon_independent_channel_check $rssi
    fi
    # We explicitly ignore clearly bogus values. 0 Mbps seen in
    # some instances where the STA is not associated by the time the Rate
    # check is done.
    if [ "$rate" -gt 0 ]; then
        if [ "$rate_num" -lt "$rate_samples" ]; then
            __repacd_wifimon_debug "Rate sample #$rate_num = $rate Mbps"

            # Ignore the very first sample since it is taken at the same time
            # the ping is started (and thus the Rate might not have been
            # updated).
            if [ "$rate_num" -eq 0 ]; then
                rate_filename=`mktemp /tmp/repacd-rate.XXXXXX`
                bestap_filename=`mktemp /tmp/repacd-bestap.XXXXXX`
                bestap_otherband_filename=`mktemp /tmp/repacd-bestap-otherband.XXXXXX`
            else
                # Not the first sample
                echo $rate >> $rate_filename
                echo $bestap >> $bestap_filename
                echo $bestap_otherband >> $bestap_otherband_filename
            fi
        elif [ "$rate_num" -eq "$rate_samples" ]; then
            __repacd_wifimon_debug "Rate sample #$rate_num = $rate Mbps"

            # We will take one more sample and then draw the conclusion.
            # No further measurements will be taken (although this may be
            # changed in the future).
            echo $rate >> $rate_filename
            echo $bestap >> $bestap_filename
            echo $bestap_otherband >> $bestap_otherband_filename

            # We got the required number of samples, now derive the median rate.
            local rate_median
            __repacd_wifimon_compute_median $rate_filename $rate_num rate_median
            __repacd_wifimon_debug "Median Rate = $rate_median Mbps"
            if [ "$device_type" = 'RE' ]; then
                if [ "$rate_median" -lt "$rate_min" ]; then
                    __repacd_wifimon_compute_median $bestap_filename $rate_num bestap_median
                    __repacd_wifimon_compute_median $bestap_otherband_filename $rate_num bestap_otherband_median
                    eval "$2=$WIFIMON_STATE_RE_MOVE_CLOSER"
                elif [ "$rate_median" -gt "$rate_max" ]; then
                    __repacd_wifimon_compute_median $bestap_filename $rate_num bestap_median
                    __repacd_wifimon_compute_median $bestap_otherband_filename $rate_num bestap_otherband_median
                    eval "$2=$WIFIMON_STATE_RE_MOVE_FARTHER"
                else
                    eval "$2=$WIFIMON_STATE_RE_LOCATION_SUITABLE"
                fi
            fi

            if [ "$rate_median" -lt "$rate_pref2G" ]; then
                if [ -z "$badlink_start_time_5g" ]; then
                    __repacd_wifimon_get_timestamp badlink_start_time_5g
                fi
            else
                # We detected good link in 5G, mark as appropriate.
                # So, next time 5G will be enabled by default.
                config_load repacd
                config_get nolink_detected_5g 'WiFiLink' 'NoLinkDetected5G' '0'
                if [ "$nolink_detected_5g" -gt 0 ]; then
                    uci_set repacd WiFiLink 'NoLinkDetected5G' '0'
                    uci_commit repacd
                fi

                badlink_start_time_5g=''
            fi

            # If 5G link is very bad and below the prefered limit,
            # switch to 2.4G backhaul link and wait for association
            # and then disable 5G backhaul link. 5G backhaul will be
            # back when the 2.4G link goes down.
            if [ "$rate_median" -lt "$rate_pref2G" ] \
                && __repacd_wifimon_is_timeout $badlink_start_time_5g $badlink_timeout5G; then
                if [ "$force_down_24g" -gt 0 ]; then
                    __repacd_wifimon_bring_iface_up $sta_iface_24g
                fi
                badlink_switch_inprogress=1
            fi

            # We have our measurement, so the ping is no longer needed.
            __repacd_stop_ping

            # In case we disassociate after this, we will want to start the
            # association timer again, so clear our state of the last time we
            # started it so that it can be started afresh upon disassociation.
            assoc_start_time=''
        fi
        rate_num=$((rate_num + 1))
    fi
}

# Measure the RSSI to the serving AP and update the state accordingly.
# input: $1 - network: the name of the network being monitored
# output: $2 - state: the variable to update with the new state name (if there
#                     was a change)
__repacd_wifimon_measure_link() {
    local rssi

    if ! __repacd_is_gw_reachable; then
        if [ -n "$last_ping_gw_ip" ]; then
            __repacd_wifimon_debug "GW ${last_ping_gw_ip} not reachable"
        else
            __repacd_wifimon_debug "GW unknown"
        fi
        return
    fi

    if [ "$rssi_num" -eq 0 ]; then
        if [ "$measuring_cnt" -gt 0 ]; then
            __repacd_wifimon_debug "Measurement failed attempt # $measuring_cnt"
        fi
        measuring_cnt=$((measuring_cnt + 1))
    fi

    if [ "$measuring_cnt" -gt "$measuring_attempts" ]; then
        if [ -n "$sta_iface_24g" ] &&
               __repacd_wifimon_is_assoc $sta_iface_24g ; then
            __repacd_wifimon_bring_iface_down $sta_iface_5g
            measuring_cnt=0
            eval "$2=$WIFIMON_STATE_RE_MOVE_CLOSER"
            return
        fi
    fi

    # Only the 5 GHz link is measured. This is especially done since we
    # generally cannot control which interface is used to reach upstream.
    # Generally 5 GHz will be used (per the rules to set the broadcast bit
    # and choose a default path), so we may not have any valid RSSI data on
    # 2.4 GHz.
    rssi=`iwconfig $sta_iface_5g | grep 'Signal level' | awk -F'=' '{print $3}' | awk '{print $1}'`
    if [ "$is_independent_channel_selection_enable" -gt 0 ]; then
        __repacd_wifimon_independent_channel_check $rssi
        if [ "$rssi_num" -gt "$rssi_samples" ]; then
            return 0
        fi
    fi
    # We explicitly ignore clearly bogus values. -95 dBm has been seen in
    # some instances where the STA is not associated by the time the RSSI
    # check is done. The check against 0 tries to guard against scenarios
    # where the firmware has yet to report an RSSI value (although this may
    # never happen if the RSSI gets primed through the association messaging).
    if [ "$rssi" -gt -95 -a "$rssi" -lt 0 ]; then
        if [ "$rssi_num" -lt "$rssi_samples" ]; then
            __repacd_wifimon_debug "RSSI sample #$rssi_num = $rssi dBm"

            # Ignore the very first sample since it is taken at the same time
            # the ping is started (and thus the RSSI might not have been
            # updated).
            if [ "$rssi_num" -eq 0 ]; then
                rssi_filename=`mktemp /tmp/repacd-rssi.XXXXXX`
            else
                # Not the first sample
                echo $rssi >> $rssi_filename
            fi
            rssi_num=$((rssi_num + 1))
        elif [ "$rssi_num" -eq "$rssi_samples" ]; then
            __repacd_wifimon_debug "RSSI sample #$rssi_num = $rssi dBm"

            # We will take one more sample and then draw the conclusion.
            # No further measurements will be taken (although this may be
            # changed in the future).
            echo $rssi >> $rssi_filename

            # We got the required number of samples, now derive the median rssi.
            local rssi_median
            __repacd_wifimon_compute_median $rssi_filename $rssi_num rssi_median
            __repacd_wifimon_debug "Median RSSI = $rssi_median dBm"
            measuring_cnt=0
            if [ "$device_type" = 'RE' ]; then
                if [ "$rssi_median" -lt "$rssi_far" ]; then
                    eval "$2=$WIFIMON_STATE_RE_MOVE_CLOSER"
                elif [ "$rssi_median" -gt "$rssi_near" ]; then
                    eval "$2=$WIFIMON_STATE_RE_MOVE_FARTHER"
                else
                    eval "$2=$WIFIMON_STATE_RE_LOCATION_SUITABLE"
                fi

                if [ "$rssi_median" -le "$rssi_pref2G" ] ;then
                    if [ -n "$sta_iface_24g" ] &&
                           __repacd_wifimon_is_assoc $sta_iface_24g ; then
                        __repacd_wifimon_bring_iface_down $sta_iface_5g
                    fi
                fi
            else  # must be standalone client
                if [ "$rssi_median" -lt "$rssi_min" ]; then
                    eval "$2=$WIFIMON_STATE_CL_LINK_INADEQUATE"
                elif [ "$rssi_median" -gt "$rssi_near" ]; then
                    eval "$2=$WIFIMON_STATE_CL_LINK_SUFFICIENT"
                elif __repacd_wifimon_is_cap_serving; then
                    eval "$2=$WIFIMON_STATE_CL_ACTING_AS_RE"
                else
                    eval "$2=$WIFIMON_STATE_CL_LINK_SUFFICIENT"
                fi
            fi

            rssi_num=$((rssi_num + 1))  # to prevent future samples

            # We have our measurement, so the ping is no longer needed.
            __repacd_stop_ping

            # In case we disassociate after this, we will want to start the
            # association timer again, so clear our state of the last time we
            # started it so that it can be started afresh upon disassociation.
            assoc_start_time=''
        fi
    fi
}

# Determine if a provided amount of time has elapsed.
# input: $1 - start_time: the timestamp (in seconds)
# input: $2 - duration: the amount of time to check against (in seconds)
# return: 0 on timeout; non-zero if no timeout
__repacd_wifimon_is_timeout() {
    local start_time=$1
    local duration=$2

    # Check if the amount of elapsed time exceeds the timeout duration.
    local cur_time
    __repacd_wifimon_get_timestamp cur_time
    local elapsed_time=$(($cur_time - $start_time))
    if [ "$elapsed_time" -gt $duration ]; then
        return 0
    fi

    return 1
}

# Check whether the given interface is the STA interface on the desired
# network and the desired band.
#
# For now, only the 5 GHz band is monitored.
#
# input: $1 - config: the name of the interface config section
# input: $2 - network: the name of the network to which the STA interface
#                      must belong to be matched
# output: $3 - iface: the resolved STA interface name on 2.4 GHz (if found)
# output: $4 - iface_config_name: the resolved name of the config section
#                                 for the STA interface on 2.4 GHz (if found)
# output: $5 - iface: the resolved STA interface name on 5 GHz (if found)
# output: $6 - iface_config_name: the resolved name of the config section
#                                 for the STA interface on 5 GHz (if found)
# output: $7 - unknown_ifaces: whether any Wi-Fi interfaces are as yet
#                              unknown (in terms of their interface name)
__repacd_wifimon_is_sta_iface() {
    local config="$1"
    local network_to_match="$2"
    local iface disabled mode device hwmode

    config_get network "$config" network
    config_get iface "$config" ifname
    config_get disabled "$config" disabled '0'
    config_get mode "$config" mode
    config_get device "$config" device
    config_get hwmode "$device" hwmode

    if [ "$hwmode" != "11ad" ]; then
        if [ "$network" = $network_to_match -a -n "$iface" -a "$mode" = "sta" \
            -a "$disabled" -eq 0 ]; then
            if whc_is_5g_vap $config; then
                eval "$5=$iface"
                eval "$6=$config"
            else
                eval "$3=$iface"
                eval "$4=$config"
            fi
        elif [ "$network" = $network_to_match -a "$mode" = "sta" \
            -a "$disabled" -gt 0 ]; then
            disabled_sta_iface_config_name=$config
        elif [ -z "$iface" -a "$disabled" -eq 0 ]; then
            # If an interface is showing as enabled but no name is known for it,
            # mark it as such. Without doing this, we can resolve the interface
            # names improperly.
            eval "$7=1"
        fi
    fi
}

# Initialize the sta_iface_5g variable with the STA interface that is enabled
# on the specified network (if any).
# input: $1 - network: the name of the network being managed
__repacd_wifimon_get_sta_iface() {
    unknown_ifaces=0

    config_load wireless
    config_foreach __repacd_wifimon_is_sta_iface wifi-iface $1 \
        sta_iface_24g sta_iface_24g_config_name \
        sta_iface_5g sta_iface_5g_config_name unknown_ifaces

    if [ "$unknown_ifaces" -gt 0 ]; then
        # Clear out everything because we cannot be certain we have the
        # right names (eg. interfaces may not all be up yet).
        sta_iface_24g=
        sta_iface_24g_config_name=
        sta_iface_5g=
        sta_iface_5g_config_name=
    fi
}

# Back haul  monitoring logic : in case 5G was brought down
# forcefully due to RSSI constraints and 2.4 G also went down
# and stayed of for more then 2GBackhaulSwitchDownTime sec
# bring back 5G.
# input: $1 - re_mode: Current RE mode we are operating in.
# input: $2 - re_submode: Current RE sub-mode we are operating in.
# output:None

__repacd_wifimon_evaluate_backhaul_link() {
    local re_mode=$1
    local re_submode=$2

    if [ "$force_down_5g" -gt 0 ] ; then
        if  ! __repacd_wifimon_is_assoc $sta_iface_24g;then
            if [ -z "$down_time_2g" ]; then
                __repacd_wifimon_get_timestamp down_time_2g
            fi

            if __repacd_wifimon_is_timeout $down_time_2g $config_downtime2G; then
                __repacd_wifimon_debug "Bringing up 5G: 2G backhaul went down"
                __repacd_wifimon_bring_iface_up $sta_iface_5g
                __repacd_wifimon_get_timestamp assoc_start_time
                down_time_2g=''
                badlink_switch_inprogress=0
            fi
        else
            down_time_2g=''
        fi

        if  __repacd_wifimon_is_timeout $force_down_5g_timestamp $backhaul_eval_time; then
            __repacd_wifimon_debug "Bringing up 5G:eval interval $backhaul_eval_time sec expired"
            __repacd_wifimon_bring_iface_up $sta_iface_5g
            __repacd_wifimon_get_timestamp force_down_5g_timestamp
            __repacd_wifimon_get_timestamp assoc_start_time
        fi
    elif [ "$force_down_24g" -gt 0 ]; then
        # If we are associated to CAP and 2.4G was brought down due to badlink,
        # Try enabling 2.4G after specified timeout.
        if [ "$last_assoc_state" -gt 0 ] \
            && __repacd_wifimon_is_assoc $sta_iface_5g; then
            if [ -z "$down_time_2g" ]; then
                __repacd_wifimon_get_timestamp down_time_2g
            fi

            if __repacd_wifimon_is_timeout $down_time_2g $config_eval_time24g; then
                __repacd_wifimon_debug "Bringing up 2.4G: Trying to associate"
                __repacd_wifimon_bring_iface_up $sta_iface_24g
                down_time_2g=''
            fi
        else
            down_time_2g=''
        fi
    fi
}

# Get phyrate in mbps
# input: $1 - 'interface on which phyrate is to be calculated'
# output: $2 - current maximum phyrate possible in Mbps
__repacd_wifimon_get_current_phyrate_in_mbps() {
    local  measurement_iface=$1
    local phyrate_in_mbps
    local phyrate_unit
    local padding="00000"

    # Get the phyrate and rate units of the interface.
    eval $(iwconfig $measurement_iface |grep 'Bit Rate' | awk -F':' '{print $2}' | awk '{ \
       a = $1;b = $2} END{print "phyrate_in_mbps=\""a"\"; \
       phyrate_unit=\""b"\""}')

    # If the phyrate is in Gbps, then convert it to Mbps
    if [ $phyrate_unit = "Gb/s" ]; then
       phyrate_in_mbps=$(echo $phyrate_in_mbps${padding:${#phyrate_in_mbps}} | sed 's/\.//g' | cut -c1-4)
    else
       phyrate_in_mbps=$(echo "$phyrate_in_mbps" | sed 's/\.//g' | cut -c1-3)
    fi

    eval "$2=$phyrate_in_mbps"
}


# Set rate measurement interface on which rate measurement
# is to be done.
# input: $1 - '5g' or '24g'
__repacd_wifimon_set_rate_measurement_iface() {
    local measurement_sta_iface_rate

    if [ "$1" = '5g' ]; then
        measurement_sta_iface=$sta_iface_5g
        measurement_sta_iface_config_name=$sta_iface_5g_config_name
        otherband_sta_iface=$sta_iface_24g
        otherband_sta_iface_config_name=$sta_iface_24g_config_name
    elif [ "$1" = '24g' ]; then
        measurement_sta_iface=$sta_iface_24g
        measurement_sta_iface_config_name=$sta_iface_24g_config_name
        otherband_sta_iface=$sta_iface_5g
        otherband_sta_iface_config_name=$sta_iface_5g_config_name
    fi

    # Not associated, we will not get valid rate, return
    if [ "$force_down_5g" -gt 0 ] \
        || ! __repacd_wifimon_is_assoc $sta_iface_5g; then
        return
    fi

    # Get the phyrate of the measurement sta interface.
    __repacd_wifimon_get_current_phyrate_in_mbps $measurement_sta_iface measurement_sta_iface_rate
    __repacd_wifimon_debug "Current maxrate is $measurement_sta_iface_rate"

    # Update the state variables depending upon the phyrate
    # of the associated sta interface. (Currently 5G)
    rate_min=$(($percent_rate_min5G*${measurement_sta_iface_rate}/100))
    rate_max=$(($percent_rate_max5G*${measurement_sta_iface_rate}/100))
    rate_pref2G=$(($percent_rate_pref2G*${measurement_sta_iface_rate}/100))

    __repacd_wifimon_debug "Rate thresholds Min=$rate_min Max=$rate_max Pref2G=$rate_pref2G"
}

# Check for configured BSSID in wireless file and update if required.
# Driver will arrive at the best AP based on rate and return the BSSID,
# the BSSID is written to the wireless config file using UCI.
# input: $1 - sta_iface: sta interface to configure
# input: $2 - sta_iface_config_name: config name of the sta interface
# input: $3 - new_bssid: BSSID value to configure
# output: $4 - restart wifi required or not
# output: $5 - Configuration changed
__repacd_wifimon_check_and_config_bssid() {
    local sta_iface=$1
    local sta_iface_config_name=$2
    local new_bssid=$3
    local current_bssid peer_bssid

    eval "$4=0"
    eval "$5=0"

    # If BSSID is empty, return
    if [ -z "$new_bssid" ]; then
        return
    fi

    current_bssid=`iwconfig $sta_iface | grep "Access Point" | awk -F" " '{print $6}'`

    config_load wireless
    # If supplied BSSID and configured BSSID are same, simply return.
    if __repacd_wifimon_is_peer_bssid_set $sta_iface_config_name; then
        if [ "$new_bssid" = "$peer_bssid" ]; then
            __repacd_wifimon_debug "$sta_iface: BSSID up to date"
            return
        fi
    elif [ "$new_bssid" = "$current_bssid" ]; then
        # Associated BSSID and supplied BSSID are same, just update
        # the config file, no restart required.
        uci_set wireless $sta_iface_config_name bssid $new_bssid
        uci_commit wireless
        __repacd_wifimon_debug "$sta_iface: BSSID updated, NO RESTART required"
        eval "$5=1"
        return
    fi

    # Write the new BSSID to wireless config file
    uci_set wireless $sta_iface_config_name bssid $new_bssid
    uci_commit wireless
    eval "$5=1"
    eval "$4=1"
    __repacd_wifimon_debug "$sta_iface: BSSID updated, RESTART required"
}

# Configure BSSID via wpa_cli interface of given sta interface
# and restart interface if requested.
# input: $1 - sta_iface: sta interface to configure
# input: $2 - new_bssid: BSSID value to configure
# output: $3 - restart wifi required or not
__repacd_wifimon_wpa_config_bssid() {
    local sta_iface=$1
    local new_bssid=$2
    local restart=$3

    if [ "$restart" -gt 0 ]; then
        # Restart the network with configured BSSID
        wpa_cli -p /var/run/wpa_supplicant-$sta_iface disable_network 0
        wpa_cli -p /var/run/wpa_supplicant-$sta_iface set_network 0 bssid $new_bssid
        wpa_cli -p /var/run/wpa_supplicant-$sta_iface enable_network 0
    else
        # Just configure the BSSID
        wpa_cli -p /var/run/wpa_supplicant-$sta_iface set_network 0 bssid $new_bssid
    fi
}

# Get current best AP based on Rate estimate.
# Driver will arrive at the best AP based on rate and return the BSSIDs.
# Best AP is determined on provided STA interface which will
# also return otherband BSSID of the same device (i.e. Best uplink node).
# input:  $1 - sta_iface - STA interface to get best uplink BSSID
# output: $2 - bestap BSSID
# output: $3 - bestap otherband BSSID
__repacd_wifimon_get_current_best_ap() {
    local sta_iface=$1
    local best_bssid best_otherband_bssid

    # Driver returns BSSID without separator(:), add the separator before writting
    # to config file.
    best_bssid=`iwpriv $sta_iface get_whc_bssid | awk -F":" '{print $2}' | \
       sed -e "s/\(..\)\(..\)\(..\)\(..\)\(..\)\(..\)/\1:\2:\3:\4:\5:\6/"`
    best_otherband_bssid=`iwpriv $sta_iface g_best_ob_bssid | awk -F":" '{print $2}' | \
       sed -e "s/\(..\)\(..\)\(..\)\(..\)\(..\)\(..\)/\1:\2:\3:\4:\5:\6/"`

    eval "$2=$best_bssid"
    eval "$3=$best_otherband_bssid"
}

# Configure Best AP based on Rate : for 5G only
# Driver will arrive at the best AP based on rate and return the BSSID,
# the BSSID is written to the wireless config file using UCI.
# input: None
# output: $1 - restart wifi required or not
__repacd_wifimon_config_best_ap() {
    local selected_bssid peer_bssid
    local restart=0 config_changed=0

    if [ -n "$sta_iface_5g" -a "$force_down_5g" -eq 0 ]; then
       __repacd_wifimon_debug "$measurement_sta_iface: Finding Best AP"

       eval "$1=0"

       if [ -n "$bestap_median" -a  "$bestap_median" != "00:00:00:00:00:00" ]; then
           if [ -n "$otherband_sta_iface" -a "$bestap_otherband_median" != "00:00:00:00:00:00" ] \
               || [ -z "$otherband_sta_iface" ]; then
               __repacd_wifimon_debug "Best AP = $bestap_median"
               __repacd_wifimon_check_and_config_bssid $measurement_sta_iface \
                   $measurement_sta_iface_config_name $bestap_median restart config_changed
               if [ "$restart" -eq 0 -a "$config_changed" -gt 0 ]; then
                   __repacd_wifimon_wpa_config_bssid $measurement_sta_iface $bestap_median $restart
               fi
               eval "$1=$restart"

               # Configure the otherband BSSID also.
               if [ -n "$otherband_sta_iface" ]; then
                   __repacd_wifimon_debug "Best AP otherband = $bestap_otherband_median"
                   __repacd_wifimon_check_and_config_bssid $otherband_sta_iface \
                       $otherband_sta_iface_config_name $bestap_otherband_median restart config_changed
               fi
           fi
       else
           __repacd_wifimon_debug "No valid Best AP found"
       fi
    fi
}

# Configure BSSID of Current association to the provided sta interface.
# The BSSID is arrived from iwconfig command.
# The BSSID is written to the wireless config file using UCI.
# input: $1 - sta_iface: station interface to configure current BSSID
# input: $2 - sta_iface_config_name: station interface config name
__repacd_wifimon_config_current_bssid() {
    local sta_iface=$1
    local sta_iface_config_name=$2
    local selected_bssid restart=0 config_changed=0

    if [ -n "$sta_iface" ]; then
        # Get the currently associated BSSID from iwconfig.
        selected_bssid=`iwconfig $sta_iface | grep "Access Point" | awk -F" " '{print $6}'`
        __repacd_wifimon_debug "$sta_iface: BSSID = $selected_bssid"

        if [ "$selected_bssid" != "00:00:00:00:00:00" ]; then
            __repacd_wifimon_check_and_config_bssid $sta_iface \
                $sta_iface_config_name $selected_bssid restart config_changed
        fi
        if [ "$restart" -gt 0 ]; then
            # Restart the network with configured BSSID
            __repacd_wifimon_wpa_config_bssid $sta_iface $selected_bssid $restart
        elif [ "$config_changed" -gt 0 ]; then
            __repacd_wifimon_wpa_config_bssid $sta_iface $selected_bssid $restart
        fi
    fi
}

# Configure 2.4G BSSID of CAP for the provided sta interface.
# Driver will arrive at the CAP 2.4G BSSID,
# the BSSID is written to the wireless config file using UCI.
# input: None
__repacd_wifimon_config_cap_24g_bssid() {
    local selected_bssid restart=0 config_changed=0

    if [ -n "$sta_iface_24g" ]; then
        # Driver returns BSSID without separator(:), add the separator before writting
        # to config file.
        selected_bssid=`iwpriv $sta_iface_24g g_whc_cap_bssid | awk -F":" '{print $2}' | \
        sed -e "s/\(..\)\(..\)\(..\)\(..\)\(..\)\(..\)/\1:\2:\3:\4:\5:\6/"`
        __repacd_wifimon_debug "$sta_iface_24g: CAP BSSID = $selected_bssid"

        if [ "$selected_bssid" != "00:00:00:00:00:00" ]; then
            __repacd_wifimon_check_and_config_bssid $sta_iface_24g \
                $sta_iface_24g_config_name $selected_bssid restart config_changed
        fi
        if [ "$restart" -gt 0 ]; then
            # Restart the network with configured BSSID
            __repacd_wifimon_wpa_config_bssid $sta_iface_24g $selected_bssid $restart
        elif [ "$config_changed" -gt 0 ]; then
            __repacd_wifimon_wpa_config_bssid $sta_iface_24g $selected_bssid $restart
        fi
    fi
}

# Configure BSSID for the disabled sta interface.
# Driver will arrive at the otherband BSSIDs from provided STA interface,
# the BSSID is written to the wireless config file using UCI.
# input: $1 - sta_iface: station interface on which otherband BSSIDs to be found
__repacd_wifimon_find_and_config_otherband_bssid() {
    local selected_bssid
    local sta_iface=$1

    if [ -n "$sta_iface" -a -n "$disabled_sta_iface_config_name" ]; then
        # Driver returns BSSID without separator(:), add the separator before writting
        # to config file.
        selected_bssid=`iwpriv $sta_iface g_whc_ob_bssid | awk -F":" '{print $2}' | \
        sed -e "s/\(..\)\(..\)\(..\)\(..\)\(..\)\(..\)/\1:\2:\3:\4:\5:\6/"`
        __repacd_wifimon_debug "OtherBand BSSID = $selected_bssid"

        if [ "$selected_bssid" != "00:00:00:00:00:00" ]; then
            if __repacd_wifimon_is_peer_bssid_set $disabled_sta_iface_config_name; then
                if [ "$selected_bssid" = "$peer_bssid" ]; then
                    __repacd_wifimon_debug "OtherBand BSSID up to date"
                    return
                fi
            fi

            uci_set wireless $disabled_sta_iface_config_name bssid $selected_bssid
            uci_commit wireless
            __repacd_wifimon_debug "OtherBand BSSID updated"
        else
            if __repacd_wifimon_is_peer_bssid_set $disabled_sta_iface_config_name; then
                uci delete wireless.${disabled_sta_iface_config_name}.bssid
                uci_commit wireless
            fi
        fi
    fi
}

# Initialize the Wi-Fi monitoring logic with the name of the network being
# monitored.
# input: $1 - network: the name of the network being managed
# input: $2 - cur_re_mode: the current operating range extender mode
# input: $3 - cur_re_submode: the current operating range extender submode
# input: $4 - autoconfig: whether it was an auto-config restart
# output: $5 - state: the name of the initial state
# output: $6 - new_re_mode: the resolved range extender mode
# output: $7 - new_re_submode: the resolved range extender submode
repacd_wifimon_init() {
    # Resolve the STA interfaces.
    # Here we assume that if we have the 5 GHz interface, that is sufficient,
    # as not all modes will have a 2.4 GHz interface.
    __repacd_wifimon_get_sta_iface $1
    if [ -n "$sta_iface_5g" -o -n "$sta_iface_24g" ]; then
        local re_mode=$2
        local re_submode=$3

        if [ -n "$sta_iface_24g" ]; then
            __repacd_wifimon_debug "Resolved 2.4 GHz STA interface to $sta_iface_24g"
            __repacd_wifimon_debug "2.4 GHz STA interface section $sta_iface_24g_config_name"
        fi

        if [ -n "$sta_iface_5g" ]; then
            __repacd_wifimon_debug "Resolved 5 GHz STA interface to $sta_iface_5g"
            __repacd_wifimon_debug "5 GHz STA interface section $sta_iface_5g_config_name"
        fi
        # First resolve the config parameters.
        config_load repacd
        config_get device_type 'repacd' 'DeviceType' 'RE'
        config_get config_re_mode 'repacd' 'ConfigREMode' 'auto'
        config_get default_re_mode 'repacd' 'DefaultREMode' 'qwrap'
        config_get traffic_separation_enabled repacd TrafficSeparationEnabled '0'
        config_get traffic_separation_active repacd TrafficSeparationActive '0'
        config_get guest_backhaul repacd NetworkGuestBackhaulInterface '2.4G'
        config_get network_guest repacd NetworkGuest 'guest'

        config_get min_auto_mode_assoc 'WiFiLink' 'MinAssocCheckAutoMode' '5'
        config_get min_wps_assoc 'WiFiLink' 'MinAssocCheckPostWPS' '5'
        config_get min_bssid_assoc 'WiFiLink' 'MinAssocCheckPostBSSIDConfig' '5'
        config_get wps_timeout 'WiFiLink' 'WPSTimeout' '120'
        config_get assoc_timeout 'WiFiLink' 'AssociationTimeout' '300'
        config_get rssi_samples 'WiFiLink' 'RSSINumMeasurements' '5'
        config_get rssi_far 'WiFiLink' 'RSSIThresholdFar' '-75'
        config_get rssi_near 'WiFiLink' 'RSSIThresholdNear' '-60'
        config_get rssi_min 'WiFiLink' 'RSSIThresholdMin' '-75'
        config_get rssi_pref2G 'WiFiLink' 'RSSIThresholdPrefer2GBackhaul' '-100'
        config_get config_downtime2G 'WiFiLink' '2GBackhaulSwitchDownTime' '10'
        config_get measuring_attempts 'WiFiLink' 'MaxMeasuringStateAttempts' '3'
        config_get daisy_chain 'WiFiLink' 'DaisyChain' '0'
        config_get rate_samples 'WiFiLink' 'RateNumMeasurements' '5'
        config_get percent_rate_min5G 'WiFiLink' 'RateThresholdMin5GInPercent' '40'
        config_get percent_rate_max5G 'WiFiLink' 'RateThresholdMax5GInPercent' '70'
        config_get percent_rate_pref2G 'WiFiLink' 'RateThresholdPrefer2GBackhaulInPercent' '5'
        config_get bssid_assoc_timeout 'WiFiLink' 'BSSIDAssociationTimeout' '90'
        config_get badlink_timeout5G 'WiFiLink' '5GBackhaulBadlinkTimeout' '60'
        config_get config_short_eval_time5g 'WiFiLink' '5GBackhaulEvalTimeShort' '1800'
        config_get config_long_eval_time5g  'WiFiLink' '5GBackhaulEvalTimeLong' '7200'
        config_get config_eval_time24g 'WiFiLink' '2GBackhaulEvalTime' '1800'
        config_get is_independent_channel_selection_enable 'WiFiLink' '2GIndependentChannelSelectionEnable' '0'
        config_get quality_5g_rssi_level 'WiFiLink' '2GIndependentChannelSelectionRssiLevel' '-70'
        config_get rssi_check_total_counter 'WiFiLink' '2GIndependentChannelSelectionTotalRssiCounter' '10'
        config_get rssi_idle_time 'WiFiLink' '2GIndependentChannelSelectionStartRssiCheckTime' '60'

        # Create ourselves a named pipe so we can be informed of WPS push
        # button events.
        if [ -e $WIFIMON_PIPE_NAME ]; then
            rm -f $WIFIMON_PIPE_NAME
        fi

        mkfifo $WIFIMON_PIPE_NAME

        # If already associated, go to the InProgress state.
        __repacd_wifimon_check_associated $1 $2 $3 $4 $5 $6 $7
    fi
    # Otherwise, must be operating in CAP mode.
}

# Check the status of the Wi-Fi link (WPS, association, and RSSI).
# input: $1 - network: the name of the network being managed
# input: $2 - cur_re_mode: the currently configured range extender mode
# input: $3 - cur_re_submode: the currently configured range extender sub-mode
# output: $4 - state: the name of the new state (only set upon a change)
# output: $5 - re_mode: the desired range extender mode (updated only once
#                       the link to the AP is considered stable)
# output: $6 - re_submode: the desired range extender submode (updated only once
#                       the link to the AP is considered stable)
repacd_wifimon_check() {
    if [ -n "$sta_iface_5g" -o -n "$sta_iface_24g" ]; then
        if __repacd_wifimon_check_associated $1 $2 $3 0 $4 $5 $6; then
            assoc_timeout_logged=0
            wps_timeout_logged=0
        else  # not associated
            # Check if the WPS button was pressed.
            local wps_pbc
            read -t 1 wps_pbc <>$WIFIMON_PIPE_NAME
            if [ $? -eq 0 ]; then
                eval "$4=$WIFIMON_STATE_AUTOCONFIG_IN_PROGRESS"
                wps_in_progress=1
                __repacd_wifimon_get_timestamp wps_start_time

                # Forcefully delete the BSSID from the STA interface in case
                # the new AP does not support IEEE1905.1+QCA extensions deep
                # cloning.
                __repacd_wifimon_debug "Delete BSSID for $sta_iface_5g"
                uci delete wireless.${sta_iface_5g_config_name}.bssid

                if [ -n "$sta_iface_24g" ]; then
                    __repacd_wifimon_debug "Delete BSSID for $sta_iface_24g"
                    uci delete wireless.${sta_iface_24g_config_name}.bssid
                fi

                uci commit wireless

                # Do not check the association timeout below.
                return 0
            fi

            if [ "$wps_in_progress" -gt 0 ]; then
                if __repacd_wifimon_is_timeout $wps_start_time $wps_timeout; then
                    if [ "$wps_timeout_logged" -eq 0 ]; then
                        __repacd_wifimon_debug "WPS timeout"
                        wps_timeout_logged=1
                    fi

                    eval "$4=$WIFIMON_STATE_WPS_TIMEOUT"
                fi
            else
                # if 2.4G inteface is down by independent channel selection
                # and 5G itnerface is gone, wake up the 2.4G interface
                if [ "$is_independent_channel_selection_enable" -gt 0 ]; then
                    if [ "$is_24G_down_by_independent_channel" -gt 0 ]; then
                        if __repacd_wifimon_is_assoc $sta_iface_5g; then
                           __repacd_wifimon_info "5G still alive"
                        else
                            __repacd_wifimon_bring_iface_up $sta_iface_24g
                            return 0
                        fi
                    fi
                fi
                if [ "$daisy_chain" -gt 0 ]; then
                    if __repacd_wifimon_is_timeout $assoc_start_time $bssid_assoc_timeout; then
                        local reset_bssid=0

                        # If 5G link associated to CAP and 2.4G is not associated,
                        # bring down 2.4G and proceed to link measurement.
                        if [ -n "$sta_iface_24g" ] && [ -n "$sta_iface_5g" ]; then
                            if [ "$force_down_24g" -eq 0 -a "$force_down_5g" -eq 0 ]; then
                                if __repacd_wifimon_is_assoc $sta_iface_5g; then
                                    # Bring down 2.4G backhaul, we are not associated for long time.
                                    # 5G backhaul is ready to take over.
                                    __repacd_wifimon_debug "BSSID Assoc timeout: bringing down 2.4G"
                                    __repacd_wifimon_bring_iface_down $sta_iface_24g
                                elif __repacd_wifimon_is_assoc $sta_iface_24g; then
                                    # Bring down 5G backhaul, we are not associated for long time.
                                    # 2.4G backhaul is ready to take over.
                                    __repacd_wifimon_debug "BSSID Assoc timeout: bringing down 5G"
                                    __repacd_wifimon_bring_iface_down $sta_iface_5g
                                else
                                    # 5G link not associated, try resetting BSSID.
                                    reset_bssid=1
                                fi
                            else
                                # We are already operating in single link,
                                # try resetting BSSID.
                                reset_bssid=1
                            fi
                        fi

                        if [ "$reset_bssid" -gt 0 ]; then
                            local bssid_deleted=0
                            config_load wireless

                            # Configured BSSID association timed out,
                            # the desired BSSID might have powered down or not reachable.
                            # Reset the BSSID and restart the wifi for fresh association.
                            if [ -n "$sta_iface_5g" ] \
                                && __repacd_wifimon_is_peer_bssid_set $sta_iface_5g_config_name; then
                                __repacd_wifimon_debug "BSSID assoc timed out delete BSSID for $sta_iface_5g"
                                uci delete wireless.${sta_iface_5g_config_name}.bssid
                                bssid_deleted=1
                            fi

                            # If we are reseting 5G BSSID, do it for 2.4G also,
                            # 2.4G BSSID will also change for first hop RE.
                            if [ -n "$sta_iface_24g" ] \
                                && __repacd_wifimon_is_peer_bssid_set $sta_iface_24g_config_name; then
                                __repacd_wifimon_debug "BSSID assoc timed out delete BSSID for $sta_iface_24g"
                                uci delete wireless.${sta_iface_24g_config_name}.bssid
                                bssid_deleted=1
                            fi

                            if [ "$bssid_deleted" -gt 0 ]; then
                                uci_commit wireless
                                uci_set repacd WiFiLink BSSIDResolveState 'resolving'
                                uci_commit repacd
                                eval "$4=$WIFIMON_STATE_BSSID_ASSOC_TIMEOUT"
                            fi
                        fi
                    fi
                fi

                if __repacd_wifimon_is_timeout $assoc_start_time $assoc_timeout; then
                    # If we have two STA interfaces and only 5 GHz is
                    # associated, see if mode switching is necessary for it
                    # alone. Note that here we are temporarily resetting the
                    # other interface name to ensure mode switching only
                    # considers the one that is associated.
                    #
                    # If we eventually start supporting multiple STA interfaces
                    # even in the fallback modes, we may need to make this
                    # smarter and consider both possible STA interfaces that
                    # may be associated.
                    if [ "$daisy_chain" -gt 0 ]; then
                        if [ -n "$disabled_sta_iface_config_name" ]; then
                            if [ -n "$sta_iface_5g" ]; then
                                # Even after resetting BSSIDs 5G is not associated,
                                # Now time to enable 2.4G link.
                                uci_set repacd WiFiLink NoLinkDetected5G '1'
                                uci_set repacd WiFiLink BSSIDResolveState 'resolving'
                                uci_commit repacd
                            elif [ -n "$sta_iface_24g" ]; then
                                # If we are here, then we have already tried 2.4G link.
                                # We are left with no option, link stays disconnected!.
                                # Atleast try to detect a link by switching between
                                # 5G and 2.4G links.
                                uci_set repacd WiFiLink NoLinkDetected5G '0'
                                uci_set repacd WiFiLink BSSIDResolveState 'resolving'
                                uci_commit repacd
                            fi
                        fi
                    elif [ -n "$sta_iface_24g" -a -n "$sta_iface_5g" ]; then
                        if [ "$force_down_5g" -eq 0 ] \
                            && __repacd_wifimon_is_assoc $sta_iface_5g; then
                            local tmp_sta_iface_24g=$sta_iface_24g
                            sta_iface_24g=
                            if ! __repacd_wifimon_resolve_mode $2 $3 $5 $6; then
                                # Not currently in the right mode. Restore the
                                # interface name and return to allow for a restart.
                                sta_iface_24g=$tmp_sta_iface_24g
                                return
                            fi
                        fi
                    fi

                    if [ "$assoc_timeout_logged" -eq 0 ]; then
                        __repacd_wifimon_debug "Association timeout"
                        assoc_timeout_logged=1
                    fi

                    eval "$4=$WIFIMON_STATE_ASSOC_TIMEOUT"
                fi
            fi
        fi
        __repacd_wifimon_evaluate_backhaul_link $2 $3

        if [ "$traffic_separation_enabled" -gt 0 ] &&
               [ "$traffic_separation_active" -gt 0 ]; then
            if ! __repacd_wifimon_is_assoc $sta_iface_5g || \
               ! __repacd_wifimon_is_assoc $sta_iface_24g || [ "$guest_link_override" -gt 0 ] || \
               [ "$force_down_5g" -gt 0 ] || [ "$force_down_24g" -gt 0 ]; then
                if [ -n "$sta_iface_24g" ] && [ -n "$sta_iface_5g" ]; then
                    if [ "$guest_backhaul" = '5G' ]; then
                        __repacd_wifimon_override_guest_backhaul $sta_iface_5g $sta_iface_24g
                    elif [ "$guest_backhaul" = '2.4G' ]; then
                        __repacd_wifimon_override_guest_backhaul $sta_iface_24g $sta_iface_5g
                    fi
                fi
            fi
        fi

    fi
}
# determin does it need to down the 2.4G backhaul or wake up 2.4G backhaul
repacd_wifimon_independent_channel_check() {
    local inteface_24g
    local i=0
    local macaddress
    local channel

    if [ -n "$sta_iface_24g" ]; then
        if [ "$is_independent_channel_selection_enable" -gt 0 ]; then
            __repacd_wifimon_debug "rssi_counter=$rssi_counter rssi_check_total_counter=$rssi_check_total_counter
            force_down_24g=$force_down_24g force_down_5g=$force_down_5g sta_iface_24g=$sta_iface_24g"

            if [ "$rssi_counter" -ge "$rssi_check_total_counter" ]; then
                    if [ "$is_24G_down_by_independent_channel" -eq 0 ]; then
                        #__repacd_wifimon_debug "need to down 2.4G"

                        __repacd_wifimon_debug "5G backhaul Signal is good, down the 2.4G backhaul interface and 2.4G AP VAP to do the ACS"

                        is_24G_down_by_independent_channel=1

                        ((echo td s; sleep 1) | hyt | awk -F " " '/WLAN2G/ && $4 < 255 {printf "%s \n %s \n",$3,$4}' \
	                    | sed 's/[[:space:]]//g') > /tmp/innetworkresult.txt

                        # find the AP VAP for 2.4G to down the interface first
                        # then set the channel to 0
                        # then wake up all interface
                        # in multiple VAP setting ACS will re-run when All VAPs channel is 0
                        (iwconfig | grep 'Mode:Master  Frequency:2' -B1 | sed '/wireless/d' |\
                        sed 's/IEEE.* //g' | sed 's/Mode.* //g' | sed 's/--//g' | sed 's/[[:space:]]//g' \
                        | sed '/^$/d') > /tmp/all_interface

                        exec < '/tmp/all_interface'
                        while read line
                        do
                           __repacd_wifimon_debug $line
                           inteface_24g=$line
                        done
                        # clear the in network information
                        wifitool $inteface_24g set_innetwork_2g 00:00:00:00:00:00 0
                        exec < '/tmp/innetworkresult.txt'

                        while read line
                        do
                            __repacd_wifimon_debug $line
                            if [ "$i" -eq 0 ]; then
                                macaddress=$line
                                i=$((i + 1))
                            else
                                channel=$line
                                /usr/sbin/wifitool $inteface_24g set_innetwork_2g $macaddress $channel
                                i=0
                            fi
                        done

                        wpa_cli -p /var/run/wpa_supplicant-$sta_iface_24g disable_network 0

                        exec < '/tmp/all_interface'
                        while read line
                        do
                           __repacd_wifimon_debug $line
                           ifconfig $line down
                           iwconfig $line channel 0
                        done
                        exec < '/tmp/all_interface'
                        while read line
                        do
                           ifconfig $line up
                        done
                        rm -rf /tmp/all_interface
                        rm -rf /tmp/innetworkresult.txt
                    else
                        if [ "$force_down_24g" -eq 0 ]; then
                            __repacd_wifimon_debug "5G backhaul signal is bad, up the 2.4G backhaul"
                            is_24G_down_by_independent_channel=0
                            wpa_cli -p /var/run/wpa_supplicant-$sta_iface_24g enable_network 0
                        else
                            __repacd_wifimon_debug "2.4G backhaul interface is down by anohter process don't wake up it"
                       fi
                   fi
                    rssi_counter=0
            fi
        fi
    else
        __repacd_wifimon_debug "sta_iface_24g=$sta_iface_24g is null, cannot do anything"
    fi
}
# Terminate the Wi-Fi link monitoring, cleaning up any state in preparation
# for shutdown.
repacd_wifimon_fini() {

    __repacd_wifimon_debug "sta_iface_24g=$sta_iface_24g is_24G_down_by_independent_channel=$is_24G_down_by_independent_channel force_down_24g=$force_down_24g"

    if [ -n "$sta_iface_24g" ]; then
        if [ "$is_24G_down_by_independent_channel" -gt 0 ]; then
            if [ "$force_down_24g" -eq 0 ]; then
                wpa_cli -p /var/run/wpa_supplicant-$sta_iface_24g enable_network 0
            fi
        fi
    fi
    __repacd_stop_ping
}
