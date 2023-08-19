#include "m_pd.h"

t_class *seq_class;

typedef struct seq {
  t_object x_obj;
  t_inlet *in_base;
  t_inlet *in_size;
  t_outlet *out;
  int default_base;
  int default_size;
  int base;
  int size;
  int fast;
  int index;
} t_seq;

void seq_bang (t_seq *x) {
  if (x->size > 0) {
    if (x->fast != 0) {
      for (int i = 0; i < x->size; i++) outlet_float(x->out, (float)(x->base + i));
    }
    else {
      outlet_float(x->out, (float)(x->base + x->index));
      if (++x->index == x->size) x->index = 0;
    }
  }
  else if (x->size < 0) {
    if (x->fast != 0) {
      for (int i = 0; i > x->size; i--) outlet_float(x->out, (float)(x->base + i));
    }
    else {
      outlet_float(x->out, (float)(x->base + x->index));
      if (--x->index == x->size) x->index = 0;
    }
  }
}

void seq_reset (t_seq *x) {
  x->base = x->default_base;
  x->size = x->default_size;
  x->index = 0;
}

void seq_base (t_seq *x, t_floatarg f) {
  x->base = (int)f;
  x->index = 0;
}

void seq_size (t_seq *x, t_floatarg f) {
  x->size = (int)f;
  x->index = 0;
}

void seq_fast (t_seq *x, t_floatarg f) {
  x->fast = (int)f;
  x->index = 0;
}

void *seq_new (t_floatarg base, t_floatarg size) {
  t_seq *x = (t_seq *)pd_new(seq_class);
  // leftmost inlet automatically created (and freed)
  x->in_base = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("base"));
  x->in_size = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("size"));
  x->out = outlet_new(&x->x_obj, &s_float);
  x->fast = 0;
  x->index = 0;
  x->default_base = (int)base;
  x->default_size = (int)size;
  seq_reset(x);
  return (void *)x;
}

void seq_free(t_seq *x) {
  inlet_free(x->in_base);
  inlet_free(x->in_size);
  outlet_free(x->out);
}

void seq_setup(void) {
  seq_class = class_new(gensym("seq"), (t_newmethod)seq_new, (t_method)seq_free,
    sizeof(t_seq), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addbang(seq_class, (t_method)seq_bang);
  class_addfloat(seq_class, (t_method)seq_fast);
  class_addmethod(seq_class, (t_method)seq_fast, gensym("fast"), A_DEFFLOAT, 0);
  class_addmethod(seq_class, (t_method)seq_reset, gensym("reset"), 0);
  class_addmethod(seq_class, (t_method)seq_base, gensym("base"), A_DEFFLOAT, 0);
  class_addmethod(seq_class, (t_method)seq_size, gensym("size"), A_DEFFLOAT, 0);
}
