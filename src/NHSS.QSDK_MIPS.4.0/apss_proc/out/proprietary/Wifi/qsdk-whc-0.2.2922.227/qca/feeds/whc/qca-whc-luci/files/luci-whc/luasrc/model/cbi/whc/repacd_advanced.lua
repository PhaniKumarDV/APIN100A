--[[
LuCI - Lua Configuration Interface

Copyright (c) 2015 Qualcomm Atheros, Inc.

All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary.

]]--

local m, s = ...

------------------------------------------------------------------------------
--Wi-Fi link monitoring settings
------------------------------------------------------------------------------
s = m:section(NamedSection, "WiFiLink", "WiFiLink", translate("Wi-Fi Link Monitoring"))
s.anonymous = true

vl = s:option(Value, "MinAssocCheckAutoMode", translate("Minimum association checks for auto-mode switching"))
vl.datatype = "uinteger"

vl = s:option(Value, "MinAssocCheckPostWPS", translate("Minimum association checks post Wi-Fi Protected Setup (WPS)"))
vl.datatype = "uinteger"

vl = s:option(Value, "MinAssocCheckPostBSSIDConfig", translate("Minimum association checks post BSSID Resolve complete in Daisy network"))
vl.datatype = "uinteger"

vl = s:option(Value, "WPSTimeout", translate("Wi-Fi Protected Setup (WPS) Timeout"))
vl.datatype = "uinteger"

vl = s:option(Value, "AssociationTimeout", translate("Association Timeout"))
vl.datatype = "uinteger"

vl = s:option(Value, "RSSINumMeasurements", translate("Number of RSSI measurements for coverage check"))
vl.datatype = "uinteger"

vl = s:option(Value, "RSSIThresholdFar", translate("RSSI value below which RE is considered too far from root AP"))
vl.datatype = "integer"

vl = s:option(Value, "RSSIThresholdNear", translate("RSSI value above which RE is considered too close to root AP"))
vl.datatype = "integer"

vl = s:option(Value, "RSSIThresholdMin", translate("Minimum RSSI value for a good link when operating in client mode"))
vl.datatype = "integer"

vl = s:option(Value, "2GBackhaulSwitchDownTime", translate("2.4G Backhaul Downtime, After this Backhaul switchover will start even to bad 5G Backhaul"))
vl.datatype = "integer"

vl = s:option(Value, "MaxMeasuringStateAttempts", translate("Max consectuive failed attemtps for measuring rssi on 5g before it is considered as bad "))
vl.datatype = "integer"

e = s:option(Flag, "DaisyChain", translate("RE Daisy chain topology Enable, Enables Rate measurement and Disables RSSI measurement"))
e.rmempty = false

vl = s:option(Value, "RateNumMeasurements", translate("Number of Rate measurements for coverage check"))
vl.datatype = "uinteger"

vl = s:option(Value, "RateThresholdMin5GInPercent", translate("Percentage 5G Rate below which RE is considered too far from connected AP, triggers Best AP selection"))
vl.datatype = "uinteger"

vl = s:option(Value, "RateThresholdMax5GInPercent", translate("Percentage 5G Rate above which RE is considered too close to connected AP, triggers Best AP selection"))
vl.datatype = "uinteger"

vl = s:option(Value, "RateThresholdPrefer2GBackhaulInPercent", translate("Percentage 5G Rate below which RE will start bad link timer"))
vl.datatype = "uinteger"

vl = s:option(Value, "5GBackhaulBadlinkTimeout", translate("5G bad link Timeout, After this RE will bring down 5G and run 2G"))
vl.datatype = "uinteger"

vl = s:option(Value, "BSSIDAssociationTimeout", translate("BSSID Association Timeout, after this RE will look for new AP"))
vl.datatype = "uinteger"

vl = s:option(Value, "RateScalingFactor", translate("Rate Scaling factor to use for this hop"))
vl.datatype = "uinteger"

vl = s:option(Value, "5GBackhaulEvalTimeShort", translate("Short eval time for bring back up forcibly down 5g backhaul.its used only once per power cycle."))
vl.datatype = "integer"

vl = s:option(Value, "5GBackhaulEvalTimeLong", translate("Long eval time for bring back up forcibly down 5g backhaul.its used till power cycle."))
vl.datatype = "integer"

vl = s:option(Value, "2GBackhaulEvalTime", translate("Eval time for bring back up forcibly down 2.4g backhaul"))
vl.datatype = "integer"

e = s:option(Flag, "ManageVAPInd", translate("Allows REPACD to control the VAP independent option"))
e.rmempty = false
