#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace antsim {

constexpr float PI = 3.14159265358979323846f;
constexpr int WORLD_W = 1280, WORLD_H = 720;
constexpr int GRID_W = 320, GRID_H = 180;
constexpr int ANT_COUNT = 50000;

struct Genes {
    float speed, sensorDistance, sensorAngle, turnRate, metabolism, deposit, exploration;
};

struct Ant {
    float x, y, angle, energy;
    uint32_t age;
    float fitness;
    Genes genes;
    bool carrying;
};

struct FoodPatch { float x, y, radius, amount; };

class Simulation {
public:
    std::vector<Ant> ants;
    std::vector<float> foodTrail, homeTrail;
    std::vector<FoodPatch> food;
    float nestX = WORLD_W * .5f, nestY = WORLD_H * .5f;
    float colonyFood = 4000.f;
    uint64_t tick = 0, births = 0, deaths = 0, deliveries = 0;
    uint32_t seed = 1337;

    explicit Simulation(uint32_t s = 1337) : foodTrail(GRID_W*GRID_H), homeTrail(GRID_W*GRID_H), seed(s), rng(s) {
        reset(s);
    }

    void reset(uint32_t s) {
        seed=s; rng.seed(s); tick=births=deaths=deliveries=0; colonyFood=4000.f;
        std::fill(foodTrail.begin(), foodTrail.end(), 0.f);
        std::fill(homeTrail.begin(), homeTrail.end(), 0.f);
        food.clear();
        for(int i=0;i<18;i++) spawnFoodPatch();
        ants.resize(ANT_COUNT);
        for(auto &a:ants) a = randomAnt();
    }

    void step(float dt = 1.f) {
        ++tick;
        if((tick & 3u)==0) diffuseAndEvaporate();
        if(tick % 900 == 0 && food.size()<28) spawnFoodPatch();

        for(size_t i=0;i<ants.size();++i) {
            Ant &a=ants[i];
            updateAnt(a,dt);
            if(a.energy<=0.f || a.age>18000) { a=offspring(); ++deaths; ++births; }
        }
        food.erase(std::remove_if(food.begin(),food.end(),[](const FoodPatch&p){return p.amount<=0.f;}),food.end());
    }

    bool save(const std::string& path) const {
        std::ofstream f(path,std::ios::binary); if(!f) return false;
        const uint32_t magic=0x41564F31, version=1; f.write((char*)&magic,4); f.write((char*)&version,4);
        f.write((char*)&seed,sizeof(seed)); f.write((char*)&tick,sizeof(tick)); f.write((char*)&births,sizeof(births));
        f.write((char*)&deaths,sizeof(deaths)); f.write((char*)&deliveries,sizeof(deliveries)); f.write((char*)&colonyFood,sizeof(colonyFood));
        uint64_t n=ants.size(), p=food.size(); f.write((char*)&n,8); f.write((char*)ants.data(),n*sizeof(Ant));
        f.write((char*)&p,8); f.write((char*)food.data(),p*sizeof(FoodPatch));
        f.write((char*)foodTrail.data(),foodTrail.size()*sizeof(float)); f.write((char*)homeTrail.data(),homeTrail.size()*sizeof(float));
        return bool(f);
    }

    bool load(const std::string& path) {
        std::ifstream f(path,std::ios::binary); if(!f) return false; uint32_t magic=0,v=0; f.read((char*)&magic,4); f.read((char*)&v,4);
        if(magic!=0x41564F31||v!=1) return false;
        f.read((char*)&seed,sizeof(seed)); f.read((char*)&tick,sizeof(tick)); f.read((char*)&births,sizeof(births));
        f.read((char*)&deaths,sizeof(deaths)); f.read((char*)&deliveries,sizeof(deliveries)); f.read((char*)&colonyFood,sizeof(colonyFood));
        uint64_t n=0,p=0; f.read((char*)&n,8); if(n>200000) return false; ants.resize(size_t(n)); f.read((char*)ants.data(),n*sizeof(Ant));
        f.read((char*)&p,8); if(p>10000) return false; food.resize(size_t(p)); f.read((char*)food.data(),p*sizeof(FoodPatch));
        f.read((char*)foodTrail.data(),foodTrail.size()*sizeof(float)); f.read((char*)homeTrail.data(),homeTrail.size()*sizeof(float));
        rng.seed(seed+uint32_t(tick)); return bool(f);
    }

    Genes averages() const {
        Genes g{}; for(const auto&a:ants){g.speed+=a.genes.speed;g.sensorDistance+=a.genes.sensorDistance;g.sensorAngle+=a.genes.sensorAngle;
          g.turnRate+=a.genes.turnRate;g.metabolism+=a.genes.metabolism;g.deposit+=a.genes.deposit;g.exploration+=a.genes.exploration;}
        float q=1.f/std::max<size_t>(1,ants.size()); g.speed*=q;g.sensorDistance*=q;g.sensorAngle*=q;g.turnRate*=q;g.metabolism*=q;g.deposit*=q;g.exploration*=q;return g;
    }

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> unit{0.f,1.f};
    float rnd(float a,float b){return a+(b-a)*unit(rng);}
    static float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
    static int gi(float x,float y){int gx=std::clamp(int(x*(GRID_W/float(WORLD_W))),0,GRID_W-1);int gy=std::clamp(int(y*(GRID_H/float(WORLD_H))),0,GRID_H-1);return gy*GRID_W+gx;}
    float sample(const std::vector<float>&g,float x,float y)const{return g[gi(x,y)];}
    Genes randomGenes(){ return {rnd(.65f,1.35f),rnd(9,28),rnd(.25f,.9f),rnd(.08f,.32f),rnd(.75f,1.25f),rnd(.7f,1.4f),rnd(.015f,.15f)}; }
    Ant randomAnt(){float a=rnd(-PI,PI),r=rnd(0,26);return{nestX+cosf(a)*r,nestY+sinf(a)*r,rnd(-PI,PI),rnd(650,1100),0,0,randomGenes(),false};}
    void spawnFoodPatch(){float x,y;do{x=rnd(50,WORLD_W-50);y=rnd(50,WORLD_H-50);}while(std::hypot(x-nestX,y-nestY)<150);food.push_back({x,y,rnd(10,25),rnd(800,2800)});}

    Genes mutate(const Genes&a,const Genes&b){
        auto m=[&](float x,float y,float lo,float hi){float z=(x+y)*.5f; if(unit(rng)<.32f)z*=rnd(.92f,1.08f);return clamp(z,lo,hi);};
        return {m(a.speed,b.speed,.45f,1.8f),m(a.sensorDistance,b.sensorDistance,6,42),m(a.sensorAngle,b.sensorAngle,.12f,1.25f),
                m(a.turnRate,b.turnRate,.04f,.55f),m(a.metabolism,b.metabolism,.5f,1.7f),m(a.deposit,b.deposit,.35f,2.2f),m(a.exploration,b.exploration,.003f,.35f)};
    }
    const Ant& parent(){
        size_t best=size_t(rng()%ants.size());
        for(int i=0;i<7;i++){size_t n=size_t(rng()%ants.size());if(ants[n].fitness>ants[best].fitness)best=n;} return ants[best];
    }
    Ant offspring(){const Ant&p1=parent(),&p2=parent();float a=rnd(-PI,PI),r=rnd(0,18);float ration=std::clamp(colonyFood,0.f,80.f);colonyFood-=ration;
        return{nestX+cosf(a)*r,nestY+sinf(a)*r,rnd(-PI,PI),720.f+ration*3.f,0,0,mutate(p1.genes,p2.genes),false};}

    void updateAnt(Ant&a,float dt){
        ++a.age; auto &trail=a.carrying?homeTrail:foodTrail;
        float sd=a.genes.sensorDistance, sa=a.genes.sensorAngle;
        auto sense=[&](float off){float an=a.angle+off;return sample(trail,a.x+cosf(an)*sd,a.y+sinf(an)*sd);};
        float l=sense(-sa),c=sense(0),r=sense(sa);
        if(c<l||c<r)a.angle+=(r-l>0?1.f:-1.f)*a.genes.turnRate*dt;
        a.angle+=rnd(-1,1)*a.genes.exploration*dt;
        if(a.carrying){float target=atan2f(nestY-a.y,nestX-a.x);float d=atan2f(sinf(target-a.angle),cosf(target-a.angle));a.angle+=d*.035f*dt;}
        float sp=1.45f*a.genes.speed*dt;a.x+=cosf(a.angle)*sp;a.y+=sinf(a.angle)*sp;
        if(a.x<2||a.x>WORLD_W-2){a.angle=PI-a.angle;a.x=clamp(a.x,2,WORLD_W-2);}if(a.y<2||a.y>WORLD_H-2){a.angle=-a.angle;a.y=clamp(a.y,2,WORLD_H-2);}
        auto&deposit=a.carrying?foodTrail:homeTrail; deposit[gi(a.x,a.y)]=std::min(60.f,deposit[gi(a.x,a.y)]+a.genes.deposit);
        a.energy-=.022f*a.genes.metabolism*(.5f+a.genes.speed)*dt;
        if(!a.carrying){for(auto&p:food){float dx=a.x-p.x,dy=a.y-p.y;if(dx*dx+dy*dy<p.radius*p.radius&&p.amount>0){a.carrying=true;p.amount-=1;a.angle+=PI;a.energy+=25;a.fitness+=8;break;}}}
        else {float dx=a.x-nestX,dy=a.y-nestY;if(dx*dx+dy*dy<34*34){a.carrying=false;a.angle+=PI;colonyFood+=1;a.energy+=75;a.fitness+=80;++deliveries;}}
    }

    void diffuseAndEvaporate(){
        static std::vector<float> tmp(GRID_W*GRID_H);
        auto pass=[&](std::vector<float>&g){
            for(int y=1;y<GRID_H-1;y++)for(int x=1;x<GRID_W-1;x++){int i=y*GRID_W+x;tmp[i]=(g[i]*.68f+(g[i-1]+g[i+1]+g[i-GRID_W]+g[i+GRID_W])*.08f)*.992f;}
            for(int x=0;x<GRID_W;x++) tmp[x]=tmp[(GRID_H-1)*GRID_W+x]=0;
            for(int y=0;y<GRID_H;y++) tmp[y*GRID_W]=tmp[y*GRID_W+GRID_W-1]=0;
            g.swap(tmp);
        };pass(foodTrail);pass(homeTrail);
    }
};
}
