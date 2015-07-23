/* vim:set et sts=4: */

#include <stdio.h>
#include <ibus.h>
#include "engine.h"
#include "config.h"

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

/* command line options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
	{ "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
	{ "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
	{ NULL },
};

static void
ibus_disconnected_cb (	IBusBus  *bus,
						gpointer  user_data)
{
	ibus_quit ();
}


static void
init (void)
{
	IBusEngineDesc *desc;
	IBusComponent *component;
	ibus_init ();

	component = ibus_component_new ("org.freedesktop.IBus.Fwnn",
									"FreeWnn Component",
									"0.1.0",
									"GPL",
									"Hashimoto Masahiko <hashimom@geeko.jp>",
									"https://github.com/hashimom/ibus-fwnn",
									"",
									"ibus-fwnn");
     desc = ibus_engine_desc_new (	"fwnn",
									"FreeWnn",
									"FreeWnn Component",
									"ja",
									"GPL",
									"Hashimoto Masahiko <hashimom@geeko.jp>",
									PKGDATADIR"/icons/ibus-fwnn.svg",
									"jp");
	
	ibus_component_add_engine(component, desc);
		
	printf("ibus_init end\n");
	
	bus = ibus_bus_new ();
	g_object_ref_sink(bus);
	g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);
	
	factory = ibus_factory_new (ibus_bus_get_connection (bus));
	g_object_ref_sink (factory);
	ibus_factory_add_engine (factory, "fwnn", IBUS_TYPE_FWNN_ENGINE);

	ibus_bus_request_name (bus, "org.freedesktop.IBus.Fwnn", 0);
	ibus_bus_register_component (bus, component);

}

int main(int argc, char **argv)
{
	GError *error = NULL;
	GOptionContext *context;

	/* Parse the command line */
	context = g_option_context_new ("- ibus freewnn engine");
	g_option_context_add_main_entries (context, entries, "ibus-fwnn");

	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("Option parsing failed: %s\n", error->message);
		g_error_free (error);
		return (-1);
	}

	/* Go */
	init ();
	printf("main init end-->\n");
	ibus_main ();
}
