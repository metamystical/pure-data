#include "m_pd.h"
#include <stdlib.h>
#include <time.h>

t_class *jabea_class;

typedef struct jabea_node {
  struct jabea_node *left;
  struct jabea_node *right;
  int num;
  int mute;
};

typedef struct jabea {
  t_object x_obj;
  t_inlet *in_fraction;
  t_outlet *out_series;
  t_outlet *out_array;
  float fraction;
  int fracint; // floor of fraction (int)
  int array_size;
  int *array;
  int index;
  struct jabea_node *root;
} t_jabea;

struct jabea_node *jabea_addNode (struct jabea_node *p, int n) {
  if (p == NULL) {
    p = (struct jabea_node *)getbytes(sizeof(struct jabea_node));
    p->num = n;
    p->mute = 0;
    p->left = NULL;
    p->right = NULL;
  }
  else if (n < p->num) p->left = jabea_addNode(p->left, n);
  else if (n > p->num) p->right = jabea_addNode(p->right, n);
  else if (n == p->num) p->mute = !p->mute;
  return p;
}

int jabea_getTreeSize (struct jabea_node *p) {
  if (p == NULL) return 0;
  return jabea_getTreeSize(p->left) + jabea_getTreeSize(p->right) + !p->mute;
}

void jabea_fillArray (struct jabea_node *p, t_jabea *x) {
  if (p == NULL) return;
  jabea_fillArray(p->left, x);
  if (!p->mute) x->array[x->index++] = p->num;
  jabea_fillArray(p->right, x);
}

struct jabea_node *jabea_freeTree (struct jabea_node *p) {
  if (p == NULL) return NULL;
  jabea_freeTree(p->left);
  jabea_freeTree(p->right);
  freebytes(p, sizeof(struct jabea_node));
  return NULL;
}

int jabea_freeArray (t_jabea *x) {
  if (x->array_size) freebytes(x->array, x->array_size);
  return 0;
}

int jabea_randomInt (int a, int b) { // returns random int in interval [a, b]
  double random = (double)rand() / (double)((unsigned)RAND_MAX + 1); // [0.0, 1.0)
  return (int)(random * (b - a + 1)) + a;
}

void jabea_out_array (t_jabea *x) {
  if (x->array_size == 0) {
    outlet_anything(x->out_array, gensym("set"), 0, NULL);
    return;
  }
  t_atom atoms[x->array_size];
  for (int i = 0; i < x->array_size; i++) SETFLOAT(atoms + i, (float)x->array[i]);
  outlet_list(x->out_array, &s_list, x->array_size, atoms);
}

void jabea_shuffleArray(t_jabea *x, int begin, int end) { // Fisher-Yates shuffle algorithm
  for (int i = end; i > begin; i--) { // shuffle
    int j = jabea_randomInt(begin, i);
    int tmp = x->array[i];
    x->array[i] = x->array[j];
    x->array[j] = tmp;
  }
  jabea_out_array(x);
}

void jabea_fracint (t_jabea *x) {
  x->fracint = (int)(x->fraction * x->array_size);
};

void jabea_fraction (t_jabea *x, t_floatarg f) {
  if (f > 0.5) {
    error("[jabea ]: fraction > 0.5 - setting to 0.5");
    f = 0.5;
  }
  else if (f < 0) {
    error("[jabea ]: fraction < 0 - setting to 0");
    f = 0;
  }
  x->fraction = f;
  jabea_fracint(x);
}

void jabea_bang (t_jabea *x) {
  if (!x->array_size) return;
  if (x->index == x->array_size) {
    if(x->fracint) jabea_shuffleArray(x, 0, x->array_size - x->fracint - 1); // omit last fraction
    jabea_shuffleArray(x, x->fracint, x->array_size - 1); // omit first fraction
    x->index = 0;
  }
  outlet_float(x->out_series, (float)(x->array[x->index++] % 200));
}

void jabea_reset (t_jabea *x) {
  x->root = jabea_freeTree(x->root);
  x->array_size = jabea_freeArray(x);
  jabea_out_array(x);
}

void jabea_number (t_jabea *x, t_floatarg f) {
  x->root = jabea_addNode(x->root, (int)f);
  if (x->array_size) freebytes(x->array, x->array_size);
  x->array_size = jabea_getTreeSize(x->root);
  jabea_fracint(x);
  if (!x->array_size) {
    jabea_out_array(x);
    return;
  }
  x->array = (int *)getbytes(x->array_size * sizeof(int));
  x->index = 0;
  jabea_fillArray(x->root, x);
  x->index = 0;
  jabea_shuffleArray(x, 0, x->array_size - 1); // full shuffle
}

void jabea_list (t_jabea *x, t_symbol *s, int argc, t_atom *argv) {
    for (int i = 0; i < argc; i++) jabea_number(x, atom_getfloat(argv + i));
}

void *jabea_new (t_floatarg fraction) {
  t_jabea *x = (t_jabea *)pd_new(jabea_class);
  // leftmost inlet automatically created (and freed)
  x->in_fraction = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fraction"));
  x->out_series = outlet_new(&x->x_obj, &s_float);
  x->out_array = outlet_new(&x->x_obj, &s_list);
  x->root = NULL;
  x->array_size = 0;
  jabea_fraction(x, fraction);
  jabea_reset(x);
  return (void *)x;
}

void jabea_free(t_jabea *x) {
  jabea_reset(x);
  inlet_free(x->in_fraction);
  outlet_free(x->out_series);
  outlet_free(x->out_array);
}

void jabea_setup(void) {
  jabea_class = class_new(gensym("jabea"), (t_newmethod)jabea_new, (t_method)jabea_free,
    sizeof(t_jabea), CLASS_DEFAULT, A_DEFFLOAT, 0);
  class_addbang(jabea_class, (t_method)jabea_bang);
  class_addfloat(jabea_class, (t_method)jabea_number);
  class_addlist(jabea_class, (t_method)jabea_list);
  class_addmethod(jabea_class, (t_method)jabea_number, gensym("number"), A_DEFFLOAT, 0);
  class_addmethod(jabea_class, (t_method)jabea_reset, gensym("reset"), 0);
  class_addmethod(jabea_class, (t_method)jabea_fraction, gensym("fraction"), A_DEFFLOAT, 0);
  srand((unsigned)time(NULL));
}
