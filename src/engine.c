/* vim:set et sts=4: */

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <syslog.h>
#include "engine.h"
#include "convert.h"
#include "fwnnserver.h"

typedef struct _IBusFwnnEngine IBusFwnnEngine;
typedef struct _IBusFwnnEngineClass IBusFwnnEngineClass;

struct _IBusFwnnEngine {
	IBusEngine parent;

    /* members */
    GString *preedit;
    gint cursor_pos;

    IBusLookupTable *table;
};

struct _IBusFwnnEngineClass {
	IBusEngineClass parent;
};

/* functions prototype */
static void	ibus_fwnn_engine_class_init	(IBusFwnnEngineClass	*klass);
static void	ibus_fwnn_engine_init		(IBusFwnnEngine		*engine);
static void	ibus_fwnn_engine_destroy		(IBusFwnnEngine		*engine);
static gboolean 
			ibus_fwnn_engine_process_key_event
                                            (IBusEngine             *engine,
                                             guint               	 keyval,
                                             guint               	 keycode,
                                             guint               	 modifiers);
static void ibus_fwnn_engine_focus_in    (IBusEngine             *engine);
static void ibus_fwnn_engine_focus_out   (IBusEngine             *engine);
static void ibus_fwnn_engine_reset       (IBusEngine             *engine);
static void ibus_fwnn_engine_enable      (IBusEngine             *engine);
static void ibus_fwnn_engine_disable     (IBusEngine             *engine);
static void ibus_engine_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_fwnn_engine_set_capabilities
                                            (IBusEngine             *engine,
                                             guint                   caps);
static void ibus_fwnn_engine_page_up     (IBusEngine             *engine);
static void ibus_fwnn_engine_page_down   (IBusEngine             *engine);
static void ibus_fwnn_engine_cursor_up   (IBusEngine             *engine);
static void ibus_fwnn_engine_cursor_down (IBusEngine             *engine);
static void ibus_fwnn_property_activate  (IBusEngine             *engine,
                                             const gchar            *prop_name,
                                             gint                    prop_state);
static void ibus_fwnn_engine_property_show
											(IBusEngine             *engine,
                                             const gchar            *prop_name);
static void ibus_fwnn_engine_property_hide
											(IBusEngine             *engine,
                                             const gchar            *prop_name);

static void ibus_fwnn_engine_update      (IBusFwnnEngine      *fwnn);

//static FwnnBroker *broker = NULL;
//static FwnnDict *dict = NULL;

G_DEFINE_TYPE (IBusFwnnEngine, ibus_fwnn_engine, IBUS_TYPE_ENGINE)

static void
ibus_fwnn_engine_class_init (IBusFwnnEngineClass *klass)
{
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
	
	ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_fwnn_engine_destroy;

	engine_class->process_key_event = ibus_fwnn_engine_process_key_event;
}

static void
ibus_fwnn_engine_init (IBusFwnnEngine *fwnn)
{
	int ret = -1;

	openlog("ibus-fwnn", LOG_PID, LOG_USER);
	ret = fwnnserver_open();
	if (ret == 0) {
		syslog(LOG_INFO, "ibus_fwnn_engine init OK");
	} else {
		syslog(LOG_ERR, "ibus_fwnn_engine init FAILED");
	}
	fwnn->preedit = g_string_new ("");
	fwnn->cursor_pos = 0;

	fwnn->table = ibus_lookup_table_new (9, 0, TRUE, TRUE);
	g_object_ref_sink (fwnn->table);
}

static void
ibus_fwnn_engine_destroy (IBusFwnnEngine *fwnn)
{
	int ret = -1;
	
	ret = fwnnserver_close();
	if (ret == 0) {
		syslog(LOG_INFO, "ibus_fwnn_engine destroy OK");
	} else {
		syslog(LOG_ERR, "ibus_fwnn_engine destroy FAILED");
	}
	
	if (fwnn->preedit) {
		g_string_free (fwnn->preedit, TRUE);
		fwnn->preedit = NULL;
	}

	if (fwnn->table) {
		g_object_unref (fwnn->table);
		fwnn->table = NULL;
	}
	
	((IBusObjectClass *) ibus_fwnn_engine_parent_class)->destroy ((IBusObject *)fwnn);
}

static void
ibus_fwnn_engine_update_lookup_table (IBusFwnnEngine *fwnn)
{
    gchar ** sugs;
    gint n_sug, i;
    gboolean retval;

    if (fwnn->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) fwnn);
        return;
    }

    ibus_lookup_table_clear (fwnn->table);
    
//    sugs = fwnn_dict_suggest (dict,
//                                 fwnn->preedit->str,
//                                 fwnn->preedit->len,
//                                 &n_sug);

//    if (sugs == NULL || n_sug == 0) {
//        ibus_engine_hide_lookup_table ((IBusEngine *) fwnn);
//        return;
//    }

    for (i = 0; i < n_sug; i++) {
        ibus_lookup_table_append_candidate (fwnn->table, ibus_text_new_from_string (sugs[i]));
    }

    ibus_engine_update_lookup_table ((IBusEngine *) fwnn, fwnn->table, TRUE);

//    if (sugs)
//        fwnn_dict_free_suggestions (dict, sugs);
}

static gboolean
ibus_fwnn_engine_kanren (IBusFwnnEngine *fwnn)
{
	IBusText *text;
	char *kanren_p = NULL;
	gint retval;

	kanren_p = fwnnserver_kanren(fwnn->preedit->str);
	text = ibus_text_new_from_static_string (kanren_p);
	g_string_assign (fwnn->preedit, text->text);
	ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND, 0x00FFFF, 0, fwnn->preedit->len);
	ibus_engine_update_preedit_text ((IBusEngine *)fwnn,
										text,
										fwnn->cursor_pos,
										TRUE);

	ibus_engine_hide_lookup_table ((IBusEngine *)fwnn);

	return TRUE;
}

/* commit preedit to client and update preedit */
static gboolean
ibus_fwnn_engine_commit (IBusFwnnEngine *fwnn)
{
	IBusText *text;
	
	if (fwnn->preedit->len == 0)
		return FALSE;

	text = ibus_text_new_from_static_string (fwnn->preedit->str);
	ibus_engine_commit_text ((IBusEngine *)fwnn, text);
	g_string_assign (fwnn->preedit, "");
	fwnn->cursor_pos = 0;

	ibus_fwnn_engine_update (fwnn);
	return TRUE;
}

static void
ibus_fwnn_engine_update (IBusFwnnEngine *fwnn)
{
	IBusText *text;
	gint retval;

	text = ibus_text_new_from_static_string (fwnn->preedit->str);

	text->attrs = ibus_attr_list_new ();    
	ibus_attr_list_append (text->attrs,
		ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, fwnn->preedit->len));
#if 0
	if (fwnn->preedit->len > 0) {
		retval = fwnn_dict_check (dict, fwnn->preedit->str, fwnn->preedit->len);
		if (retval != 0) {
			ibus_attr_list_append (text->attrs,
				ibus_attr_foreground_new (0xff0000, 0, fwnn->preedit->len));
		}
	}
#endif
    
	ibus_engine_update_preedit_text ((IBusEngine *)fwnn,
										text,
										fwnn->cursor_pos,
										TRUE);

	ibus_engine_hide_lookup_table ((IBusEngine *)fwnn);
}


#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z) || ((c) >= IBUS_A && (c) <= IBUS_Z))

static gboolean 
ibus_fwnn_engine_process_key_event (IBusEngine *engine,
									guint		keyval,
									guint		keycode,
									guint		modifiers)
{
	int target_len = 0;
	IBusText *text;
	IBusFwnnEngine *fwnn = (IBusFwnnEngine *)engine;

	if (modifiers & IBUS_RELEASE_MASK)
		return FALSE;

	if (modifiers != 0) {
		if (fwnn->preedit->len == 0)
			return FALSE;
		else
			return TRUE;
	}

	switch (keyval) {
	case IBUS_space:
		if (fwnn->preedit->len == 0)
			return FALSE;
		return ibus_fwnn_engine_kanren (fwnn);
	case IBUS_Return:
		if (fwnn->preedit->len == 0)
			return FALSE;
		return ibus_fwnn_engine_commit (fwnn);

	case IBUS_Escape:
		if (fwnn->preedit->len == 0)
			return FALSE;
		g_string_assign (fwnn->preedit, "");
		fwnn->cursor_pos = 0;
		ibus_fwnn_engine_update (fwnn);
		return TRUE;        

	case IBUS_Left:
		if (fwnn->preedit->len == 0)
			return FALSE;
		if (fwnn->cursor_pos > 0) {
			target_len = conv_get_bytesize_laststr(fwnn->preedit, fwnn->cursor_pos);
			fwnn->cursor_pos = fwnn->cursor_pos - target_len;
			ibus_fwnn_engine_update (fwnn);
		}
		return TRUE;

	case IBUS_Right:
		if (fwnn->preedit->len == 0)
			return FALSE;
		if (fwnn->cursor_pos < fwnn->preedit->len) {
			target_len = conv_get_bytesize_nextstr(fwnn->preedit, fwnn->cursor_pos);
			syslog(LOG_INFO, "IBUS_Right: %d + %d -> %d", fwnn->cursor_pos, target_len, (fwnn->cursor_pos + target_len));
			fwnn->cursor_pos = fwnn->cursor_pos + target_len;
			ibus_fwnn_engine_update (fwnn);
			
		}
		return TRUE;
    
	case IBUS_Up:
		if (fwnn->preedit->len == 0)
			return FALSE;
		if (fwnn->cursor_pos != 0) {
			fwnn->cursor_pos = 0;
			ibus_fwnn_engine_update (fwnn);
		}
		return TRUE;
	
	case IBUS_Down:
		if (fwnn->preedit->len == 0)
			return FALSE;
		if (fwnn->cursor_pos != fwnn->preedit->len) {
			fwnn->cursor_pos = fwnn->preedit->len;
			ibus_fwnn_engine_update (fwnn);
		}
		return TRUE;
    
	case IBUS_BackSpace:
		if (fwnn->preedit->len == 0)
			return FALSE;
		if (fwnn->cursor_pos > 0) {
			target_len = conv_get_bytesize_laststr(fwnn->preedit, fwnn->cursor_pos);
			fwnn->cursor_pos = fwnn->cursor_pos - target_len;
			g_string_erase (fwnn->preedit, fwnn->cursor_pos, target_len);
			ibus_fwnn_engine_update (fwnn);
		}
		return TRUE;
    
	case IBUS_Delete:
		if (fwnn->preedit->len == 0)
			return FALSE;
		if (fwnn->cursor_pos < fwnn->preedit->len) {
			target_len = conv_get_bytesize_nextstr(fwnn->preedit, fwnn->cursor_pos);
			g_string_erase (fwnn->preedit, fwnn->cursor_pos, target_len);
			ibus_fwnn_engine_update (fwnn);
		}
		return TRUE;
	}

	if (is_alpha (keyval)) {
		g_string_insert_c (	fwnn->preedit,
							fwnn->cursor_pos,
							keyval);
		fwnn->cursor_pos = conv_run_romajiconv(fwnn->preedit, fwnn->cursor_pos);
		ibus_fwnn_engine_update (fwnn);
        
		return TRUE;
	}

    return FALSE;
}
