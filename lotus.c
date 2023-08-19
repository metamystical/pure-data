#include "m_pd.h"
#include <stdlib.h>
#include <string.h>

#define LOTUS_FILENAME "db"
#define LOTUS_STRING 100

t_class *lotus_class;

typedef struct lotus_node {
  struct lotus_node *left;
  struct lotus_node *right;
  char *key;
  char *val;
  int mute;
};

typedef struct lotus {
  t_object x_obj;
  int pathlength;
  char *filepath;
  t_float delete;
  t_inlet *in_delete;
  t_outlet *out_message;
  struct lotus_node *root;
} t_lotus;

struct lotus_node *lotus_addNode (struct lotus_node *p, char *key, char *val) {
  if (p == NULL) {
    p = (struct lotus_node *)getbytes(sizeof(struct lotus_node));
    p->key = key;
    p->val = val;
    p->mute = 0;
    p->left = NULL;
    p->right = NULL;
  }
  else {
    int cmp = strcmp(p->key, key);
    if (cmp > 0) p->left = lotus_addNode(p->left, key, val);
    else if (cmp < 0) p->right = lotus_addNode(p->right, key, val);
    else {
      freebytes(p->val, strlen(p->val) + 1);
      p->val = val;
      p->mute = 0;
    }
  }
  return p;
}

char *lotus_getVal (struct lotus_node *p, char *key, int delete) {
  if (p == NULL) return NULL;
  int cmp = strcmp(p->key, key);
  if (cmp > 0) return lotus_getVal(p->left, key, delete);
  else if (cmp < 0) return lotus_getVal(p->right, key, delete);
  else {
    if (p->mute) return NULL;
    if (delete) p->mute = 1;
    return p->val;
  }
}

void lotus_printTree (struct lotus_node *p, FILE *fp) {
  if (p == NULL) return;
  lotus_printTree(p->left, fp);
  if (!p->mute) {
    if (fp == NULL) post("%s %s", p->key, p->val);
    else fprintf(fp, "%s %s\n", p->key, p->val);
  }
  lotus_printTree(p->right, fp);
}

void lotus_freeTree (struct lotus_node *p) {
  if (p == NULL) return;
  lotus_freeTree(p->left);
  lotus_freeTree(p->right);
  freebytes(p->key, strlen(p->key) + 1);
  freebytes(p->val, strlen(p->val) + 1);
  freebytes(p, sizeof(struct lotus_node));
}

void report (char *path) {
  error("unable to open database file: %s", path);
}

void lotus_save (t_lotus *x) {
  FILE *fp;
  if (fp = sys_fopen(x->filepath, "w")) {
    lotus_printTree(x->root, fp);
    sys_fclose(fp);
    post("data saved to file: %s", x->filepath);
  }
  else report(x->filepath);
}

void lotus_read (t_lotus *x) {
  FILE *fp;
  char *line = NULL;
  ssize_t read;
  size_t len = 0;
  char *key, *val, *val2;

  if (fp = sys_fopen(x->filepath, "r")) {
    while ((read = getline(&line, &len, fp)) != -1) {
      if (line[--read] != '\0') line[read] = '\0';
      val = strchr(line, ' ');
      if (val != NULL) {
        *val = '\0';
        ++val;
        key = getbytes(strlen(line) + 1);
        strcpy(key, line);
        val2 = getbytes(strlen(val) + 1);
        strcpy(val2, val);
        x->root = lotus_addNode(x->root, key, val2);
      }
    }
    free(line);
    sys_fclose(fp);
  }
  else report(x->filepath);
}

void lotus_bang (t_lotus *x) {
  lotus_printTree(x->root, NULL);
}

void lotus_query (t_lotus *x, t_symbol *s, int argc, t_atom *argv) {
  t_binbuf *bs = binbuf_new();
  if (strcmp(s->s_name, "symbol") == 0) {
    s = atom_getsymbol(argv);
    char string[LOTUS_STRING];
    snprintf(string, LOTUS_STRING, "%s", s->s_name); // unescape
    binbuf_text(bs, string, strlen(string));
    argv = binbuf_getvec(bs);
    argc = binbuf_getnatom(bs);
    char select[LOTUS_STRING];
    atom_string(argv, select, LOTUS_STRING);
    s = gensym(select);
    if (--argc > 0) argv += 1;
  }
  else if (argc != 0 && ((strcmp(s->s_name, "list") == 0) || (strcmp(s->s_name, "float") == 0))) {
    char key[LOTUS_STRING];
    atom_string(argv, key, LOTUS_STRING);
    s = gensym(key);
    if (--argc > 0) argv += 1;
  }
  if (argc == 0) {
    char *val = lotus_getVal(x->root, s->s_name, x->delete);
    if (val == NULL) error("key not found: %s", s->s_name);
    else {
      if (x->delete) {
        post("key removed: %s", s->s_name);
        lotus_save(x);
      }
      else {
        t_binbuf *bb = binbuf_new();
        binbuf_text(bb, val, strlen(val));
        t_atom *vec = binbuf_getvec(bb);
        int num = binbuf_getnatom(bb);
        char select[LOTUS_STRING];
        atom_string(vec, select, LOTUS_STRING);
        if (num > 0) outlet_anything(x->out_message, gensym(select), num - 1, vec + 1);
        binbuf_free(bb);
      }
    }
  }
  else {
    t_binbuf *bb = binbuf_new();
    binbuf_add(bb, argc, argv);
    char *val;
    int len;
    binbuf_gettext(bb, &val, &len); // no null term
    val = (char *)resizebytes(val, len, len + 1);
    val[len] = '\0';
    x->root = lotus_addNode(x->root, s->s_name, val);
    binbuf_free(bb);
    lotus_save(x);
  }
  binbuf_free(bs);
}
  
void *lotus_new (t_symbol *filename) {
  t_lotus *x = (t_lotus *)pd_new(lotus_class);
  // leftmost inlet automatically created (and freed)
  if (strlen(filename->s_name) == 0) filename = gensym(LOTUS_FILENAME);
  x->root = NULL;
  x->delete = 0;
  x->pathlength = strlen(canvas_getcurrentdir()->s_name) + strlen(filename->s_name) + 2;
  x->filepath = getbytes(x->pathlength);
  strcpy(x->filepath, canvas_getcurrentdir()->s_name);
  strcat(x->filepath, "/");
  strcat(x->filepath, filename->s_name);
  lotus_read(x);

  x->in_delete = floatinlet_new(&x->x_obj, &x->delete);
  x->out_message = outlet_new(&x->x_obj, 0);
  return (void *)x;
}

void lotus_free(t_lotus *x) {
  freebytes(x->filepath, x->pathlength);
  lotus_freeTree(x->root);
  inlet_free(x->in_delete);
  outlet_free(x->out_message);
}

void lotus_setup(void) {
  lotus_class = class_new(gensym("lotus"), (t_newmethod)lotus_new, (t_method)lotus_free,
    sizeof(t_lotus), CLASS_DEFAULT, A_DEFSYMBOL, 0);
  class_addbang(lotus_class, (t_method)lotus_bang);
  class_addanything(lotus_class, (t_method)lotus_query);
}
