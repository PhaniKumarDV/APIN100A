--[[
Copyright (c) 2016 Qualcomm Atheros, Inc.
All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary

Index of the menus for the Streamboost Darmok applicaiton.

Source proprietary to Qualcomm Atheros, Inc. 2016
$Id$
]]--

module("luci.controller.atf.atf", package.seeall)

function index()
    entry({"admin", "atf"}, template("atf/atfconfig"), _("Airtime"), 75).index = true
    entry({"admin", "atf", "atfconfig"}, template("atf/atfconfig"), "Fairness", 1).dependent=false
    entry({"admin", "atf", "atfview"}, template("atf/atfview"), "Performance", 2).dependent=false
    entry({"admin", "atf", "showatftable"}, template("atf/showatftable"))
    entry({"admin", "atf", "flushatftable"}, template("atf/flushatftable"))
    entry({"admin", "atf", "showairtime"}, template("atf/showairtime"))
    entry({"admin", "atf", "addssid"}, template("atf/addssid"))
    entry({"admin", "atf", "delssid"}, template("atf/delssid"))
    entry({"admin", "atf", "addsta"}, template("atf/addsta"))
    entry({"admin", "atf", "delsta"}, template("atf/delsta"))
    entry({"admin", "atf", "addatfgroup"}, template("atf/addatfgroup"))
    entry({"admin", "atf", "delatfgroup"}, template("atf/delatfgroup"))
    entry({"admin", "atf", "configatfgroup"}, template("atf/configatfgroup"))
    entry({"admin", "atf", "showatfgroup"}, template("atf/showatfgroup"))
    entry({"admin", "atf", "setatf"}, template("atf/setatf"))
    entry({"admin", "atf", "atfstatus"}, template("atf/atfstatus"))
    entry({"admin", "atf", "iwconfig"}, template("atf/iwconfig"))
    entry({"admin", "atf", "strict"}, template("atf/strict"))
    entry({"admin", "atf", "setstrict"}, template("atf/setstrict"))
    entry({"admin", "atf", "liststa"}, template("atf/liststa"))
    entry({"admin", "atf", "leases"}, template("atf/leases"))
end
