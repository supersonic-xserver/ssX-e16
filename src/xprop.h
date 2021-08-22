/*
 * Copyright (C) 2004-2021 Kim Woelders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _XPROP_H_
#define _XPROP_H_

#include "xtypes.h"

EX_Atom             ex_atom_get(const char *name);
void                ex_atoms_get(const char *const *names,
				 unsigned int num, EX_Atom * atoms);

int                 ex_client_message32_send(EX_Window win, EX_Atom type,
					     unsigned int mask, unsigned int d0,
					     unsigned int d1, unsigned int d2,
					     unsigned int d3, unsigned int d4);

void                ex_window_prop_del(EX_Window win, EX_Atom atom);

void                ex_window_prop_card32_set(EX_Window win, EX_Atom atom,
					      const unsigned int *val,
					      unsigned int num);
int                 ex_window_prop_card32_get(EX_Window win, EX_Atom atom,
					      unsigned int *val,
					      unsigned int len);
int                 ex_window_prop_card32_list_get(EX_Window win, EX_Atom atom,
						   unsigned int **plst);

void                ex_window_prop_xid_set(EX_Window win, EX_Atom atom,
					   EX_Atom type,
					   const EX_ID * lst, unsigned int num);
int                 ex_window_prop_xid_get(EX_Window win, EX_Atom atom,
					   EX_Atom type,
					   EX_ID * lst, unsigned int len);
int                 ex_window_prop_xid_list_get(EX_Window win, EX_Atom atom,
						EX_Atom type, EX_ID ** plst);
void                ex_window_prop_xid_list_change(EX_Window win, EX_Atom atom,
						   EX_Atom type,
						   EX_ID item, int op);
void                ex_window_prop_atom_set(EX_Window win, EX_Atom atom,
					    const EX_Atom * val,
					    unsigned int num);
int                 ex_window_prop_atom_get(EX_Window win, EX_Atom atom,
					    EX_Atom * val, unsigned int len);
int                 ex_window_prop_atom_list_get(EX_Window win, EX_Atom atom,
						 EX_Atom ** plst);
void                ex_window_prop_atom_list_change(EX_Window win, EX_Atom atom,
						    EX_Atom item, int op);
void                ex_window_prop_window_set(EX_Window win, EX_Atom atom,
					      const EX_Window * val,
					      unsigned int num);
int                 ex_window_prop_window_get(EX_Window win, EX_Atom atom,
					      EX_Window * val,
					      unsigned int len);
int                 ex_window_prop_window_list_get(EX_Window win, EX_Atom atom,
						   EX_Window ** plst);

void                ex_window_prop_string_set(EX_Window win, EX_Atom atom,
					      const char *str);
char               *ex_window_prop_string_get(EX_Window win, EX_Atom atom);

/* Misc atoms */

typedef struct {
#define DEFINE_ATOM_MISC(a) EX_Atom a;
#include "xpropdefs.h"
#undef DEFINE_ATOM_MISC
} e_atoms_misc_t;

extern e_atoms_misc_t ea_m;

void                ex_atoms_init(void);

/* ICCCM */

typedef struct {
#define DEFINE_ATOM_ICCCM(a) EX_Atom a;
#include "xpropdefs.h"
#undef DEFINE_ATOM_ICCCM
} e_atoms_icccm_t;

extern e_atoms_icccm_t ea_i;

void                ex_icccm_init(void);

void                ex_icccm_delete_window_send(EX_Window win, EX_Time ts);
void                ex_icccm_take_focus_send(EX_Window win, EX_Time ts);

void                ex_icccm_title_set(EX_Window win, const char *title);
char               *ex_icccm_title_get(EX_Window win);
void                ex_icccm_name_class_set(EX_Window win,
					    const char *name, const char *clss);
void                ex_icccm_name_class_get(EX_Window win,
					    char **name, char **clss);

/* NETWM (EWMH) */

typedef struct {
#define DEFINE_ATOM_NETWM(a) EX_Atom a;
#include "xpropdefs.h"
#undef DEFINE_ATOM_NETWM
} e_atoms_netwm_t;

extern e_atoms_netwm_t ea_n;

void                ex_netwm_init(void);

void                ex_netwm_wm_identify(EX_Window root,
					 EX_Window check, const char *wm_name);

void                ex_netwm_desk_count_set(EX_Window root,
					    unsigned int n_desks);
void                ex_netwm_desk_roots_set(EX_Window root,
					    const EX_Window * vroots,
					    unsigned int n_desks);
void                ex_netwm_desk_names_set(EX_Window root,
					    const char **names,
					    unsigned int n_desks);
void                ex_netwm_desk_size_set(EX_Window root,
					   unsigned int width,
					   unsigned int height);
void                ex_netwm_desk_workareas_set(EX_Window root,
						const unsigned int *areas,
						unsigned int n_desks);
void                ex_netwm_desk_current_set(EX_Window root,
					      unsigned int desk);
void                ex_netwm_desk_viewports_set(EX_Window root,
						const unsigned int *origins,
						unsigned int n_desks);
void                ex_netwm_showing_desktop_set(EX_Window root, int on);

void                ex_netwm_client_list_set(EX_Window root,
					     const EX_Window * p_clients,
					     unsigned int n_clients);
void                ex_netwm_client_list_stacking_set(EX_Window root,
						      const EX_Window *
						      p_clients,
						      unsigned int n_clients);
void                ex_netwm_client_active_set(EX_Window root, EX_Window win);
void                ex_netwm_name_set(EX_Window win, const char *name);
int                 ex_netwm_name_get(EX_Window win, char **name);
void                ex_netwm_icon_name_set(EX_Window win, const char *name);
int                 ex_netwm_icon_name_get(EX_Window win, char **name);
void                ex_netwm_visible_name_set(EX_Window win, const char *name);
int                 ex_netwm_visible_name_get(EX_Window win, char **name);
void                ex_netwm_visible_icon_name_set(EX_Window win,
						   const char *name);
int                 ex_netwm_visible_icon_name_get(EX_Window win, char **name);

void                ex_netwm_desktop_set(EX_Window win, unsigned int desk);
int                 ex_netwm_desktop_get(EX_Window win, unsigned int *desk);

int                 ex_netwm_user_time_get(EX_Window win, unsigned int *ts);

void                ex_netwm_opacity_set(EX_Window win, unsigned int opacity);
int                 ex_netwm_opacity_get(EX_Window win, unsigned int *opacity);

void                ex_netwm_startup_id_set(EX_Window win, const char *id);
int                 ex_netwm_startup_id_get(EX_Window win, char **id);

void                ex_icccm_state_set_iconic(EX_Window win);
void                ex_icccm_state_set_normal(EX_Window win);
void                ex_icccm_state_set_withdrawn(EX_Window win);

void                ex_window_prop_string_list_set(EX_Window win, EX_Atom atom,
						   char **lst, int num);
int                 ex_window_prop_string_list_get(EX_Window win, EX_Atom atom,
						   char ***plst);

#endif /* _XPROP_H_ */
