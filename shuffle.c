#include "m_pd.h"
#include <stdlib.h>
#include <time.h>

t_class *shuffle_class;

typedef struct shuffle {
  t_object x_obj;
  t_inlet *in_lower, *in_upper, *in_fraction;
  t_outlet *out_series;
  int lower;
  int upper;
  float fraction;
  int fracint; // floor of fraction (int)
  int array_size;
  int *array;
  int index;
} t_shuffle;

int shuffle_randomInt (int a, int b) { // returns random int in interval [a, b]
  double random = (double)rand() / (double)((unsigned)RAND_MAX + 1); // [0.0, 1.0)
  return (int)(random * (b - a + 1)) + a;
}

void shuffle_array(t_shuffle *x, int begin, int end) { // Fisher–Yates shuffle algorithm
  for (int i = end; i > begin; i--) { // shuffle
    int j = shuffle_randomInt(begin, i);
    int tmp = x->array[i];
    x->array[i] = x->array[j];
    x->array[j] = tmp;
  }
}

void shuffle_reset (t_shuffle *x) {
  if (x->upper < x->lower) {
    error("[shuffle ]: upper < lower - swapping");
    int tmp = x->upper;
    x->upper = x->lower;
    x->lower = tmp;
  }
  if (x->fraction > 0.5) {
    error("[shuffle ]: fraction > 0.5 - setting to 0.5");
    x->fraction = 0.5;
  }
  else if (x->fraction < 0) {
    error("[shuffle ]: fraction < 0 - setting to 0");
    x->fraction = 0;
  }
  x->array_size = x->upper - x->lower + 1;
  x->index = 0;
  x->array = (int *)getbytes(x->array_size * sizeof(int));
  x->fracint = (int)(x->fraction * x->array_size);
  for (int i = 0; i < x->array_size; i++) x->array[i] = x->lower + i;
  shuffle_array(x, 0, x->array_size - 1); // full shuffle
}

void shuffle_bang (t_shuffle *x) {
  if (x->index == x->array_size) {
    if(x->fracint) shuffle_array(x, 0, x->array_size - x->fracint - 1); // omit last fraction
    shuffle_array(x, x->fracint, x->array_size - 1); // omit first fraction
    x->index = 0;
  }
  outlet_float(x->out_series, (float)x->array[x->index++]);
}

void shuffle_lower (t_shuffle *x, t_floatarg f) {
  x->lower = (int)f;
  freebytes(x->array, x->array_size);
  shuffle_reset(x);
}

void shuffle_upper (t_shuffle *x, t_floatarg f) {
  x->upper = (int)f;
}

void shuffle_fraction (t_shuffle *x, t_floatarg f) {
  x->fraction = f;
}

void *shuffle_new (t_floatarg lower, t_floatarg upper, t_floatarg fraction) {
  t_shuffle *x = (t_shuffle *)pd_new(shuffle_class);
  // leftmost inlet automatically created (and freed)
  x->in_upper = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("upper"));
  x->in_fraction = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fraction"));
  x->out_series = outlet_new(&x->x_obj, &s_float);
  x->lower = (int)lower;
  x->upper = (int)upper;
  x->fraction = fraction;
  shuffle_reset(x);
  return (void *)x;
}

void shuffle_free(t_shuffle *x) {
  freebytes(x->array, x->array_size);
  inlet_free(x->in_upper);
  inlet_free(x->in_fraction);
  outlet_free(x->out_series);
}

void shuffle_setup(void) {
  shuffle_class = class_new(gensym("shuffle"), (t_newmethod)shuffle_new, (t_method)shuffle_free,
    sizeof(t_shuffle), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(shuffle_class, (t_method)shuffle_bang);
  class_addfloat(shuffle_class, (t_method)shuffle_lower);
  class_addmethod(shuffle_class, (t_method)shuffle_lower, gensym("lower"), A_FLOAT, 0);  
  class_addmethod(shuffle_class, (t_method)shuffle_upper, gensym("upper"), A_FLOAT, 0);  
  class_addmethod(shuffle_class, (t_method)shuffle_fraction, gensym("fraction"), A_FLOAT, 0);  
  srand((unsigned)time(NULL));  
}
