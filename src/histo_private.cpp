#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <thread>
#include <vector>
#include <iostream>
#include "Timer.h"

extern "C" {
#include "ppmb_io.h"
}

using namespace std;

class parthist{
	public:
		parthist(int n)
		{
			hist_r = (int *) calloc(n+1, sizeof(int));
			hist_g = (int *) calloc(n+1, sizeof(int));
			hist_b = (int *) calloc(n+1, sizeof(int));
		};

		void histr(int n)
		{
			hist_r[n]+=1;
		}
		void histg(int n)
		{
			hist_g[n]+=1;
		}
		void histb(int n)
		{
			hist_b[n]+=1;
		}
		int getr(int i)
		{
			return hist_r[i];
		}
		int getg(int i)
		{
			return hist_g[i];
		}
		int getb(int i)
		{
			return hist_b[i];
		}
	private:
		int *hist_r;
		int *hist_g;
		int *hist_b;
};

struct img {
  int xsize;
  int ysize;
  int maxrgb;
  unsigned char *r;
  unsigned char *g;
  unsigned char *b;
};

void print_histogram(FILE *f, int *hist, int N) {
  fprintf(f, "%d\n", N+1);
  for(int i = 0; i <= N; i++) {
    fprintf(f, "%d %d\n", i, hist[i]);
  }
}

void parth(struct img *input,parthist *m, int n,int i)
{
	int seg = input->xsize * input->ysize/n;
	int end;
	if(i == n-1)
		end = input->xsize * input->ysize;
	else end = seg*(i+1);
	for(int pix = seg*i;pix < end;pix++)
	{
		m->histr(input->r[pix]);
		m->histg(input->g[pix]);
		m->histb(input->b[pix]);
	}
}

void histogram(struct img *input, int *hist_r, int *hist_g, int *hist_b,int n) {
  // we assume hist_r, hist_g, hist_b are zeroed on entry.

  // for(int pix = 0; pix < input->xsize * input->ysize; pix++) {
  //   hist_r[input->r[pix]] += 1;
  //   hist_g[input->g[pix]] += 1;
  //   hist_b[input->b[pix]] += 1;
  // }
	thread threads[n];
	vector<parthist> parts;
	for(int i = 0;i<n;i++)
	{
		parts.push_back(parthist(input->maxrgb));
	}

	for(int i = 0;i<n;i++)
	{

		threads[i] = thread(parth,input,&parts[i],n,i);
	}
	for(int i = 0;i<n;i++)
	{
		threads[i].join();
	}
	//cout << parts[1].getr(1) << endl;
	// merge
	for(int i =0;i<=input->maxrgb;i++)
	{
		for(int j = 0;j<n;j++)
		{
			hist_r[i]+=parts[j].getr(i);
			hist_g[i]+=parts[j].getg(i);
			hist_b[i]+=parts[j].getb(i);
		}
	}

}

int main(int argc, char *argv[]) {
  if(argc != 4) {
    printf("Usage: %s input-file output-file threads\n", argv[0]);
    printf("       For single-threaded runs, pass threads = 1\n");
    exit(1);
  }

  char *output_file = argv[2];
  char *input_file = argv[1];
  int threads = atoi(argv[3]);

  // /* remove this in multithreaded version */
  // if(threads != 1) {
  //   printf("ERROR: Only supports single-threaded execution\n");
  //   exit(1);
  // }

  struct img input;

  if(!ppmb_read(input_file, &input.xsize, &input.ysize, &input.maxrgb,
		&input.r, &input.g, &input.b)) {
    if(input.maxrgb > 255) {
      printf("Maxrgb %d not supported\n", input.maxrgb);
      exit(1);
    }

    int *hist_r, *hist_g, *hist_b;

    hist_r = (int *) calloc(input.maxrgb+1, sizeof(int));
    hist_g = (int *) calloc(input.maxrgb+1, sizeof(int));
    hist_b = (int *) calloc(input.maxrgb+1, sizeof(int));

    ggc::Timer t("histogram");

    t.start();
    histogram(&input, hist_r, hist_g, hist_b, threads);
    t.stop();


    FILE *out = fopen(output_file, "w");
    if(out) {
      print_histogram(out, hist_r, input.maxrgb);
      print_histogram(out, hist_g, input.maxrgb);
      print_histogram(out, hist_b, input.maxrgb);
      fclose(out);
    } else {
      fprintf(stderr, "Unable to output!\n");
    }
    printf("Time: %llu ns\n", t.duration());
  }
}
