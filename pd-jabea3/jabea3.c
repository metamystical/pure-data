#include "m_pd.h"
#include <stdlib.h>
#include <time.h>

/* PCG32 Minimal Implementation - Public Domain (by M.E. O'Neill) */
/* http://www.pcg-random.org */

#include <stdint.h>

typedef struct {
    uint64_t state;
    uint64_t inc;  // Stream identifier (can be fixed or variable)
} pcg32_random_t;

static inline uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    // Calculate output function (XSH RR), uses old state
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

static inline void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq) {
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg32_random_r(rng);  // Advance once
    rng->state += initstate;
    pcg32_random_r(rng);  // Advance again
}

uint32_t jabea3_random (pcg32_random_t* rng, uint32_t bound) {
  // returns random int in interval [0, bound)
  uint32_t range = UINT32_MAX / bound;
  uint32_t limit = range * bound;
  uint32_t r;
  do {
    r = pcg32_random_r(rng);
  } while (r >= limit);
  return r / range;
}

t_class *jabea3_class;

struct jabea3 {
  t_object x_obj;
  t_outlet *out_series;
  t_outlet *out_array;
  int array_size;
  int *array;
  int toggle;
  pcg32_random_t rng;
};

void jabea3_outArray (struct jabea3 *x) {
  if (x->array_size == 0) {
    outlet_anything(x->out_array, gensym("set"), 0, NULL);
    return;
  }
  t_atom atoms[x->array_size];
  for (int i = 0; i < x->array_size; i++) SETFLOAT(atoms + i, (float)x->array[i]);
  outlet_list(x->out_array, &s_list, x->array_size, atoms);
}

void jabea3_freeArray (struct jabea3 *x) {
  if (x->array_size) freebytes(x->array, x->array_size);
  x->array_size = 0;
}

void jabea3_loadArray(struct jabea3 *x, int argc, int *argv) {
  jabea3_freeArray(x);
  x->array_size = argc;
  x->array = (int *)getbytes(argc * sizeof(int));
  for (int i = 0; i < argc; i++) x->array[i] = argv[i];
  jabea3_outArray(x);
}

void jabea3_toggle(struct jabea3 *x, t_floatarg f)
{
    x->toggle = (f != 0);
}

void jabea3_reset (struct jabea3 *x) {
  jabea3_freeArray(x);
  jabea3_outArray(x);
}

void jabea3_list (struct jabea3 *x, t_symbol *s, int argc, t_atom *argv) {
  if (argc == 0) return;
  if (!x->toggle) { // add notes to beginning of buffer
    int notes_size = x->array_size + argc;
    int notes[notes_size];
    for (int i = 0; i < argc; i++) notes[i] = atom_getint(argv + i);
    for (int i = 0; i < x->array_size; i++) notes[i + argc] = x->array[i];
    jabea3_loadArray(x, notes_size, notes);
    return;
  } // remove notes from buffer
  int notes_size = x->array_size;
  int notes[notes_size];
  for (int i = 0; i < notes_size; i++) notes[i] = x->array[i];
  for (int i = 0; i < argc; i++) {
    int note = atom_getint(argv + i);
    // remove first matching note from left in buffer, or continue
    int j = 0;
    while (j < notes_size) {
      if (note == notes[j]) break;
      ++j;
    }
    if (j == notes_size) continue;
    int m = 0;
    for (int k = 0; k < notes_size ; k++) {
      if (k == j) m = 1;
      notes[k] = notes[k + m];
    }
    --notes_size;
  }
  jabea3_loadArray(x, notes_size, notes);
}

void jabea3_number (struct jabea3 *x, t_floatarg f) {
  t_atom list[1];
  SETFLOAT(&list[0], f);
  jabea3_list(x, &s_list, 1, list);
}

void jabea3_bang (struct jabea3 *x) {
  if (!x->array_size) return;
  outlet_float(x->out_series, (float)(x->array[jabea3_random(&x->rng, x->array_size)]));
}

void *jabea3_new () {
  struct jabea3 *x = (struct jabea3 *)pd_new(jabea3_class);
  // leftmost inlet automatically created (and freed)
  x->out_series = outlet_new(&x->x_obj, &s_float);
  x->out_array = outlet_new(&x->x_obj, &s_list);
  x->array_size = 0;
  jabea3_reset(x);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("toggle"));
  x->toggle = 0;
  pcg32_srandom_r(&x->rng, (uint64_t)time(NULL), 0xda3e39cb94b95bdbULL);
  return (void *)x;
}

void jabea3_free(struct jabea3 *x) {
  jabea3_freeArray(x);
}

void jabea3_setup(void) {
  jabea3_class = class_new(gensym("jabea3"), (t_newmethod)jabea3_new, (t_method)jabea3_free,
    sizeof(struct jabea3), CLASS_DEFAULT, A_DEFFLOAT, 0);
  class_addbang(jabea3_class, (t_method)jabea3_bang);
  class_addfloat(jabea3_class, (t_method)jabea3_number);
  class_addlist(jabea3_class, (t_method)jabea3_list);
  class_addmethod(jabea3_class, (t_method)jabea3_toggle, gensym("toggle"), A_DEFFLOAT, 0);
  class_addmethod(jabea3_class, (t_method)jabea3_reset, gensym("reset"), 0);
}
