#include <wiringPi.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <deque>

#include "json.hpp"


using json = nlohmann::json;
using namespace std;

map<string, int>  get_fs_conf()
{
  map<string, int> fs_conf { 
    {"on-threshold", 60}, 
    {"off-threshold", 50},
    {"budget",3},
    {"delay", 10}
  };

  try
  {
    ifstream fs_cfg_file("/usr/local/etc/fanshim.json");
    json fs_cfg_custom;
    fs_cfg_file >> fs_cfg_custom;

    for (auto& el : fs_cfg_custom.items()) {
      fs_conf[el.key()] = el.value();
    }
  }
  catch (...)
  {
    cout<<"error parsing config file"<<endl;
  }

  for (map<string,int>::iterator it=fs_conf.begin(); it!=fs_conf.end(); ++it)
    cout << it->first << " => " << it->second << endl;

  return fs_conf;
}


int main (void)
{

  const int fanshim_pin = 18;

  wiringPiSetupGpio();
  pinMode(fanshim_pin, OUTPUT);

  map<string, int> fs_conf = get_fs_conf();

  const int sleep_msec = fs_conf["delay"]*1000*1000;
  const int on_threshold = fs_conf["on-threshold"];
  const int off_threshold = fs_conf["off-threshold"];
  const int budget = fs_conf["budget"];

  int read_fs_pin = 0;

  const string node_hdr = "# HELP cpu_fanshim text file output: fan state.\n# TYPE cpu_fanshim gauge\ncpu_fanshim ";
  const string node_hdr_t = "# HELP cpu_temp_fanshim text file output: temp.\n# TYPE cpu_temp_fanshim gauge\ncpu_temp_fanshim ";
  string nodex_out = "";

  fstream tmp_file;
  float tmp = 0;
  deque<int> tmp_q (budget,0.0);
  int j;
  bool all_low,all_high;

  tmp_file.open("/sys/class/thermal/thermal_zone0/temp", ios_base::in);

  while(1){
    tmp_file >> tmp;
    tmp_file.seekg(0, tmp_file.beg);
    tmp = tmp/1000;
    tmp_q.push_back(int(tmp));
    tmp_q.pop_front();
    deque<int> (tmp_q).swap(tmp_q);

    cout<<"Temp: "<<tmp<<", last "<<budget<<": [ ";
    for (j =0; j<tmp_q.size(); j++)
    {
      cout << tmp_q.at(j) <<" ";
    }
    cout<<"]\n";

    all_low = all_of(tmp_q.begin(), tmp_q.end(), [=](int tx){return tx<off_threshold;});
    all_high = all_of(tmp_q.begin(), tmp_q.end(), [=](int tx){return tx>on_threshold;});

    cout<<"all low: "<< boolalpha << all_low <<endl;
    cout<<"all high: "<< boolalpha << all_high <<endl;

    read_fs_pin = digitalRead(fanshim_pin);
    if(all_high && read_fs_pin == 0)
    {
      digitalWrite(fanshim_pin, 1);
    }
    else 
    {
      if(all_low && read_fs_pin == 1)
      {
        digitalWrite(fanshim_pin, 0);
      }
    }


    read_fs_pin = digitalRead(fanshim_pin);
    cout<<"fan state now: "<< (read_fs_pin == 0 ? "[off]" : "[on]") <<endl;

    ofstream nodex_fs;
    nodex_fs.open("/usr/local/etc/node_exp_txt/cpu_fan.prom");
    nodex_out = node_hdr + to_string(read_fs_pin) + "\n";
    nodex_out += node_hdr_t + to_string(int(tmp)) + "\n";
    nodex_fs<<nodex_out;
    nodex_fs.close();

    usleep(sleep_msec);
  }

  return 0 ;

}
