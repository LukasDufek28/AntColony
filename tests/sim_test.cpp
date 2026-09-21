#include "../src/simulation.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
int main(){
  antsim::Simulation s(42); assert(s.ants.size()==antsim::ANT_COUNT);
  auto begin=std::chrono::steady_clock::now(); for(int i=0;i<120;i++)s.step();
  auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-begin).count();
  assert(s.tick==120); assert(s.save("test-world.ants")); antsim::Simulation t; assert(t.load("test-world.ants")); assert(t.tick==s.tick);
  std::cout<<"ants="<<s.ants.size()<<" ticks=120 ms="<<ms<<" deliveries="<<s.deliveries<<"\n";
}
