#pragma once
#include <cstdlib>
#include <iostream> 

bool RuleExists(const std::string& ruleName) {
    std::string cmd = "netsh advfirewall firewall show rule name=\"" + ruleName + "\" >nul 2>&1";
    return (system(cmd.c_str()) == 0);
}

// Tambahin port TCP/UDP jika belum ada
void SetupFirewallAndPorts() {
    if (!RuleExists("3000"))
        system("netsh advfirewall firewall add rule name=\"3000\" dir=in action=allow protocol=TCP localport=3000");

    if (!RuleExists("80"))
        system("netsh advfirewall firewall add rule name=\"80\" dir=in action=allow protocol=TCP localport=80");

    if (!RuleExists("443"))
        system("netsh advfirewall firewall add rule name=\"443\" dir=in action=allow protocol=TCP localport=443");

    if (!RuleExists("17091"))
        system("netsh advfirewall firewall add rule name=\"17091\" dir=in action=allow protocol=UDP localport=17091");
}
inline void RemoveFirewallRules() {
    system("netsh advfirewall firewall delete rule name=\"TCP_3000\"");
    system("netsh advfirewall firewall delete rule name=\"TCP_80\"");
    system("netsh advfirewall firewall delete rule name=\"TCP_443\"");
    system("netsh advfirewall firewall delete rule name=\"UDP_17091\"");
}