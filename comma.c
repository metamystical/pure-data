#include <string.h>
#include "m_pd.h"

t_class *comma_class;

typedef struct comma {
  t_object x_obj;
  t_outlet *out_comma;
  t_atom *atoms;
  int num;
} t_comma;

void comma_free_atoms (t_comma *x) {
  if (x->num) freebytes(x->atoms, x->num);
  x->num = 0;
  x->atoms = NULL;
}

void comma_add_atom (t_comma *x, t_atom atom) {
  size_t N = sizeof(t_atom);
  if (x->num == 0) x->atoms = (t_atom *)getbytes(++(x->num) * N);
  else x->atoms = (t_atom *)resizebytes(x->atoms, x->num * N, ++(x->num) * N);
  x->atoms[x->num - 1] = atom;
}

void comma_bang (t_comma *x) {
  comma_free_atoms(x);
  outlet_anything(x->out_comma, gensym("set"), x->num, x->atoms);
}

void comma_anything (t_comma *x, t_symbol *s, int argc, t_atom *argv) {
  if (argc == 1) comma_add_atom(x, argv[0]);
  else if (argc > 1) {
    if (strcmp(s->s_name, "list") != 0) return;
    comma_free_atoms(x);
    t_atom comma;
    SETCOMMA(&comma);
    for (int i = 0; i < argc; i++) {
      comma_add_atom(x, argv[i]);
      if (i < argc - 1) comma_add_atom(x, comma);
    }
  }
  outlet_anything(x->out_comma, gensym("set"), x->num, x->atoms);
}

void *comma_new (t_symbol *s) {
  t_comma *x = (t_comma *)pd_new(comma_class);
  // leftmost inlet automatically created (and freed)
  x->out_comma = outlet_new(&x->x_obj, 0);
  x->num = 0;
  x->atoms = NULL;
  return (void *)x;
}

void comma_free(t_comma *x) {
  comma_free_atoms(x);
  outlet_free(x->out_comma);
}

void comma_setup(void) {
  comma_class = class_new(gensym("comma"), (t_newmethod)comma_new, (t_method)comma_free,
    sizeof(t_comma), CLASS_DEFAULT, A_DEFSYMBOL, 0);
  class_addbang(comma_class, (t_method)comma_bang);
  class_addanything(comma_class, (t_method)comma_anything);
}
