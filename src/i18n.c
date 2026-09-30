#define _GNU_SOURCE

#include "i18n.h"

#include "boxinfo.h"

#include <locale.h>
#include <stdio.h>
#include <string.h>

#define SETTINGS "/etc/enigma2/settings"

static char current[32] = "en_US";
static int generation;

/* The value of key in the settings of Enigma2, 0 when missing. */
static int setting(const char *key, char *value, size_t size)
{
	char line[256];
	size_t length = strlen(key);
	int found = 0;
	FILE *file = fopen(SETTINGS, "r");
	if (!file)
		return 0;
	while (fgets(line, sizeof(line), file))
		if (strncmp(line, key, length) == 0 && line[length] == '=') {
			text_cut(line, "\r\n");
			snprintf(value, size, "%s", line + length + 1);
			found = value[0] != '\0';
		}
	fclose(file);
	return found;
}

/* Like Enigma2: config.osd.language, else config.misc.locale, else the default of the brand. */
static void language(char *locale, size_t size)
{
	char brand[64];
	const char *fallback = "de_DE";
	if (setting("config.osd.language", locale, size) || setting("config.misc.locale", locale, size))
		return;
	boxinfo_value("displaybrand", brand, sizeof(brand));
	if (!strcmp(brand, "Atto.TV"))
		fallback = "pt_BR";
	else if (!strcmp(brand, "Zgemma"))
		fallback = "en_US";
	else if (!strcmp(brand, "Beyonwiz"))
		fallback = "en_AU";
	snprintf(locale, size, "%s", fallback);
}

int i18n_set(const char *locale)
{
	/* Only the messages: numbers and times keep the C format, e.g. in the socket log. */
	if (!setlocale(LC_MESSAGES, locale))
		return 0;
	snprintf(current, sizeof(current), "%s", locale);
	generation++;
	return 1;
}

const char *i18n_locale(void)
{
	return current;
}

int i18n_generation(void)
{
	return generation;
}

int i18n_rtl(void)
{
	return !strncmp(current, "ar", 2) || !strncmp(current, "fa", 2) || !strncmp(current, "he", 2);
}

void i18n_init(void)
{
	char locale[32];
	bindtextdomain("orm", LOCALEDIR);
	bind_textdomain_codeset("orm", "UTF-8");
	textdomain("orm");
	language(locale, sizeof(locale));
	if (!i18n_set(locale))
		i18n_set("C");
}
