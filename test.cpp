/* ########################################################################### */
/* ########################################################################### */

#include <stdint.h>           // for int8_t, uint32_t
#include <stdio.h>            // for printf
#include <cstdlib>            // for exit
#include <regex>              // for regex_search, match_results<>::_Base_type
#include <string>             // for string, stoi, operator!=
#include <vector>             // for vector
#include "plug_controller.h"  // for PlugController

/* ########################################################################### */
/* ########################################################################### */

void loop();
void setup();
void parseEnergyStats(std::string p);
void parseSysinfo(std::string p);

/* ########################################################################### */
/* ########################################################################### */

PlugController energyPlug;
PlugController lampPlug;

/* ########################################################################### */
/* ########################################################################### */

void setup()
{
    energyPlug = PlugController("192.168.0.40", 9999);
    lampPlug = PlugController("192.168.0.28", 9999);
}

void loop()
{
    lampPlug.on();
    lampPlug.countDown(5, false);
    lampPlug.setLedOff();

    std::string response = lampPlug.getInfo();
    if (response != "")
        parseSysinfo(response);

    response = energyPlug.getInfo();
    if (response != "")
        parseSysinfo(response);

    response = energyPlug.getEmeter();
    if (response != "")
        parseEnergyStats(response);

    lampPlug.setLedOn();
    exit(0);
}

/* ########################################################################### */

void parseSysinfo(std::string p)
{
    const std::regex regex_state("relay_state\":([0-9]),");
    const std::regex regex_on_time("on_time\":([0-9]+),");
    const std::regex regex_alias("alias\":\"(.*?)\",");

    std::smatch matches;
    std::string mstring;

    regex_search(p, matches, regex_state);
    if (matches.empty())
        return;
    std::string relay_state = matches[1];

    regex_search(p, matches, regex_on_time);
    if (matches.empty())
        return;
    std::string on_time = matches[1];

    std::string alias = "Unknown";
    regex_search(p, matches, regex_alias);
    if (!matches.empty())
        alias = matches[1];

    int8_t i_state = std::stoi(relay_state);
    uint32_t i_ontime = std::stoi(on_time);

    printf("\nAlias: %s", alias.c_str());
    printf("\n");

    if (i_state == 0)
        printf("Relay is off\n");
    else if (i_state == 1)
        printf("Relay is on for %ss\n", on_time.c_str());
    else
        printf("Relay is in unknown state\n");
}

/* ########################################################################### */

void parseEnergyStats(std::string p)
{
    const std::regex regex_power("power_mw\":([0-9]+),");
    const std::regex regex_total("total_wh\":([0-9]+),");

    std::smatch matches;
    std::string mstring;

    regex_search(p, matches, regex_power);
    if (matches.empty())
        return;
    std::string power_mw = matches[1];

    regex_search(p, matches, regex_total);
    if (matches.empty())
        return;
    std::string total_wh = matches[1];

    int int_power = std::stoi(power_mw);
    int int_total = std::stoi(total_wh);

    double power = (double)int_power / 1000;
    double total_kwh = (double)int_total / 1000;

    printf("\n\nPower: %f Watt\nTotal: %f kWh\n\n", power, total_kwh);

    /*     std::string cmd = "rrdtool update rrd/nrg.rrd N:";
    cmd += std::to_string(power);
    cmd += ":";
    cmd += std::to_string(total_kwh);

    exec(cmd.c_str()); */
}

/* ########################################################################### */

int main(int argc, char *argv[])
{
    setup();
    while (1)
        loop();
    return 0;
}
