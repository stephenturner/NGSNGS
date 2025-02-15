#include <stdlib.h>
#include <random>
#include <iostream>
#include <climits>
#include "mrand.h"
#include <cassert>

mrand_t *mrand_alloc(int type_a,long int seedval){
  mrand_t *ret = (mrand_t *) malloc(sizeof(mrand_t));
  ret->type = type_a;

#ifdef __APPLE__
  if(ret->type==0){
    fprintf(stderr,"\t-> Problem with drand48 reentrant, will default to erand48\n");
    ret->type = 3;
  }
#else
  if(ret->type==0)
    srand48_r(seedval,(struct drand48_data *) &ret->buf0);
#endif
  if(ret->type==1){
    ret->eng = std::default_random_engine(seedval);
    ret->distr = std::uniform_real_distribution<double>(0, 1);
    ret->distrInt = std::uniform_int_distribution<>(0,INT_MAX);
  }
  if(ret->type==2)
    ret->rand_r_seed = (unsigned int) seedval;
  if(ret->type==3){
    ret->rand_r_seed = (unsigned int) seedval;
    ret->xsubi[2] = ret->rand_r_seed >> 16;
    ret->xsubi[1] = ret->rand_r_seed & 0xffffl;
    ret->xsubi[0] = 0x330e;
  }
  return ret;
}

double mrand_pop(mrand_t *mr){
  double res;
  if(mr->type==0){
    #if defined(__linux__) || defined(__unix__)
    drand48_r((struct drand48_data*)&mr->buf0,&res);
    #endif
  }
  else if(mr->type==1){
    res =  mr->distr(mr->eng);
  }
  else if(mr->type==2){
    int randr = rand_r(&mr->rand_r_seed);
    if (randr == RAND_MAX)
      res = (double) (randr-1)/RAND_MAX;     
    else
      res = (double) randr/RAND_MAX;
  }
  else if(mr->type==3){
    res = erand48(mr->xsubi);
  }
  else{
    fprintf(stderr,"Random parameter %d is not supported\n",mr->type);
    exit(0);
  }
  assert(res!=0.0);
  assert(res!=1.0);
  return res;
}
long mrand_pop_long(mrand_t *mr){
  long res;
  if(mr->type==0){
    #if defined(__linux__) || defined(__unix__)
    lrand48_r((struct drand48_data*)&mr->buf0,&res);
    #endif
  }
  else if(mr->type==1){
    res =  mr->distrInt(mr->eng);
  }
  else if(mr->type==2){
    res = rand_r(&mr->rand_r_seed) ;
  }
  else if(mr->type==3){
    res = abs(jrand48(mr->xsubi));
  }
  else{
    fprintf(stderr,"Random parameter %d is not supported\n",mr->type);
    exit(0);
  }
  return res;
}


int Random_geometric_k(const double p,mrand_t *mr)
{
  double u = mrand_pop(mr);
  int k;

  if (p == 1){k = 1;}
  else if(p == 0){k=0;}
  else{k = log (u) / log (1 - p);}

  return floor(k);
}


#ifdef __WITH_MAIN__

int main(int argc,char **argv){
  mrand_t *myrand;
  int type =0;
  int seed =1;
  long long nitems =10;
  double dogeom = 0.0;
  if(argc>3){
    type = atoi(argv[1]);
    seed = atoi(argv[2]);
    nitems = atoll(argv[3]);
  }
  if(argc>4)
    dogeom = atof(argv[4]);
  fprintf(stderr,"type: %d seed: %d nitems: %lld dogeom: %f\n",type,seed,nitems,dogeom);
  myrand = mrand_alloc(type,seed);

  if(dogeom==0){
    double sum = 0;
    for(long long i=0;i<nitems;i++){
      sum += mrand_pop(myrand);
    }
    fprintf(stdout,"type %d\tseed: %d\tsum:%f\tnitems in mio:%f mean: %f\n",type,seed,sum,nitems/1e6,sum/((double)nitems));
  }else{
    for(long long i=0;i<nitems;i++)
      fprintf(stdout,"%d\n",Random_geometric_k(dogeom,myrand));
  }
  
  return 0;
}

#endif

//g++ mrand.cpp -std=c++11 -lm -lz -D__WITH_MAIN__ -o Rand
