#include "m_pd.h"

t_class *memorex_class;

typedef struct memorex {
  t_object x_obj;
  t_inlet *in_replay;
  t_outlet *out_note, *out_chord;
  int size, **array, index, count;
  int fifo, replay, num_notes;
} t_memorex;

void memorex_freeArray (t_memorex *x) {
  for (int i = 0; i < x->size; i++) freebytes(x->array[i], x->num_notes * sizeof(int));
  freebytes(x->array, x->size * sizeof(int *));
}

void memorex_init (t_memorex *x) {
  if (x->size >= 0) x->fifo = 1;
  else {
    x->size = -x->size;
    x->fifo = 0;
  }
  if (x->size) {
    x->array = (int **)getbytes(x->size * sizeof(int *));
    for (int i = 0; i < x->size; i++) x->array[i] = (int *)getbytes(x->num_notes * sizeof(int));
  }
  x->index = -1;
  x->count = 0;
}

void memorex_num_notes (t_memorex *x, t_floatarg g) {
  int c = (int)g;
  if (c <= 0) c = 1;
  x->num_notes = c;
  if (x->size) {
    memorex_freeArray(x);
    memorex_init(x);
  }
}

void memorex_size (t_memorex *x, t_floatarg f) {
  if (x->size) memorex_freeArray(x);
  x->size = (int)f;
  memorex_init(x);
}

void memorex_output(t_memorex *x) {
  outlet_float(x->out_note, (float)x->array[x->index][0]);
  t_atom atoms[x->num_notes];
  for (int i = 0; i < x->num_notes; i++) SETFLOAT(atoms + i, (float)x->array[x->index][i]);
  outlet_anything(x->out_chord, gensym("set"), x->num_notes, atoms);
}

void memorex_chord (t_memorex *x, t_symbol *s, int argc, t_atom *argv) {
  if (!x->size || argc == 0) return;
  if (x->replay) {
    if (!x->count) return;
    if (x->fifo) {
      if (++x->index == x->count) x->index = 0;
      memorex_output(x);
    }
    else {
      memorex_output(x);
      if (--x->index == -1) x->index = x->count - 1;
    }
  }
  else {
    if (++x->index == x->size) x->index = 0;
    if (x->index == x->count && x->count < x->size) ++x->count;
    for (int i = 0; i < x->num_notes; i++) x->array[x->index][i] = i < argc ? atom_getint(argv + i) : 0;
    memorex_output(x);
  }
}

void memorex_note (t_memorex *x, t_floatarg f) {
  t_atom atom;
  SETFLOAT(&atom, f);
  memorex_chord(x, &s_list, 1, &atom);
}

void memorex_replay (t_memorex *x, t_floatarg f) {
  if (f == 0) x->replay = 0;
  else x->replay = 1;
}

void *memorex_new (t_floatarg f, t_floatarg g) {
  t_memorex *x = (t_memorex *)pd_new(memorex_class);
  // leftmost inlet automatically created (and freed)
  x->in_replay = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("replay"));
  x->out_note = outlet_new(&x->x_obj, &s_float);
  x->out_chord = outlet_new(&x->x_obj, 0);
  x->size = 0;
  memorex_num_notes(x, g);
  memorex_size(x, f);
  x->replay = 0;
  return (void *)x;
}

void memorex_free(t_memorex *x) {
  if (x->size) memorex_freeArray(x);
  inlet_free(x->in_replay);
  outlet_free(x->out_note);
  outlet_free(x->out_chord);
}

void memorex_setup(void) {
  memorex_class = class_new(gensym("memorex"), (t_newmethod)memorex_new, (t_method)memorex_free,
    sizeof(t_memorex), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addfloat(memorex_class, (t_method)memorex_note);
  class_addlist(memorex_class, (t_method)memorex_chord);
  class_addmethod(memorex_class, (t_method)memorex_note, gensym("note"), A_DEFFLOAT, 0);
  class_addmethod(memorex_class, (t_method)memorex_replay, gensym("replay"), A_DEFFLOAT, 0);
  class_addmethod(memorex_class, (t_method)memorex_size, gensym("size"), A_DEFFLOAT, 0);
  class_addmethod(memorex_class, (t_method)memorex_num_notes, gensym("num_notes"), A_DEFFLOAT, 0);
}
