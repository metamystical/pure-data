#include "m_pd.h"

t_class *seld_class;

typedef struct seld {
  t_object x_obj;
  t_inlet *in_inx;
  t_inlet *in_val;
  t_outlet **out_bang;
  t_outlet *out_dump;
  t_outlet *out_args;
  int inx;
  int num_vals;
  t_float *vals;
} t_seld;

void seld_input (t_seld *x, t_floatarg f) {
  int dump = 1;
  for (int i = 0; i < x->num_vals; i++) {
    if (x->vals[i] == f) {
      outlet_bang(x->out_bang[i]);
      dump = 0;
    }
  }
  if (dump) outlet_float(x->out_dump, f);
}

void seld_inx (t_seld *x, t_floatarg f) {
  int inx = (int)f;
  if (inx < 0) inx = -inx;
  x->inx = inx % x->num_vals;
}

void seld_out_args (t_seld *x) {
  t_atom atoms[x->num_vals];
  for (int i = 0; i < x->num_vals; i++) SETFLOAT(atoms + i, x->vals[i]);
  outlet_list(x->out_args, &s_list, x->num_vals, atoms);
}

void seld_val (t_seld *x, t_floatarg f) {
  x->vals[x->inx] = f;
  seld_out_args(x);
}

void seld_list (t_seld *x, t_symbol *s, int argc, t_atom *argv) {
  for (int i = 0; i < argc && i < x->num_vals; i++) x->vals[i] = atom_getfloat(argv + i);
  seld_out_args(x);
}

void seld_bang (t_seld *x) {
  seld_out_args(x);
}

void *seld_new (t_symbol *s, int argc, t_atom *argv) {
  t_seld *x = (t_seld *)pd_new(seld_class);
  // leftmost inlet automatically created (and freed)
  x->in_inx = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("index"));
  x->in_val = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("value"));
  x->out_args = outlet_new(&x->x_obj, &s_list);
  x->out_dump = outlet_new(&x->x_obj, &s_float);
  x->num_vals = argc;
  x->vals = (t_float *)getbytes(argc * sizeof(t_float));
  x->out_bang = (t_outlet **)getbytes(argc * sizeof(t_outlet *));
  for (int i = 0; i < argc; i++) {
    x->vals[i] = atom_getfloat(argv + i);
    x->out_bang[i] = outlet_new(&x->x_obj, &s_bang);
  }
  return (void *)x;
}

void seld_free(t_seld *x) {
  inlet_free(x->in_inx);
  inlet_free(x->in_val);
  outlet_free(x->out_args);
  for (int i = 0; i < x->num_vals; i++)outlet_free(x->out_bang[i]);
  freebytes(x->vals, x->num_vals * sizeof(t_float));
  freebytes(x->out_bang, x->num_vals * sizeof(t_outlet *));
}

void seld_setup(void) {
  seld_class = class_new(gensym("seld"), (t_newmethod)seld_new, (t_method)seld_free,
    sizeof(t_seld), CLASS_DEFAULT, A_GIMME, 0);
  class_addbang(seld_class, (t_method)seld_bang);
  class_addfloat(seld_class, (t_method)seld_input);
  class_addlist(seld_class, (t_method)seld_list);
  class_addmethod(seld_class, (t_method)seld_inx, gensym("index"), A_DEFFLOAT, 0);
  class_addmethod(seld_class, (t_method)seld_val, gensym("value"), A_DEFFLOAT, 0);
}
