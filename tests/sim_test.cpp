#include "../src/simulation.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
int main(){
  antsim::Simulation s(42); assert(s.ants.size()==antsim::ANT_COUNT);
  auto begin=std::chrono::steady_clock::now(); for(int i=0;i<600;i++)s.step();
  auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-begin).count();
  double meanDistance=0; size_t carrying=0; for(const auto&a:s.ants){meanDistance+=std::hypot(a.x-s.nestX,a.y-s.nestY);carrying+=a.carrying;}meanDistance/=s.ants.size();
  assert(s.tick==600); assert(meanDistance>150); assert(s.save("test-world.ants")); antsim::Simulation t; assert(t.load("test-world.ants")); assert(t.tick==s.tick);
  std::cout<<"ants="<<s.ants.size()<<" ticks=600 ms="<<ms<<" mean_distance="<<meanDistance<<" carrying="<<carrying<<" deliveries="<<s.deliveries<<"\n";
}
