#define _GNU_SOURCE

#include "boxinfo.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void text_cut(char *text, const char *chars)
{
	char *end = strpbrk(text, chars);
	if (end)
		*end = '\0';
}

void boxinfo_value(const char *key, char *value, size_t size)
{
	char line[256];
	size_t length = strlen(key);
	FILE *file = fopen("/usr/lib/enigma.info", "r");
	value[0] = '\0';
	if (!file)
		return;
	while (fgets(line, sizeof(line), file))
		if (strncmp(line, key, length) == 0 && line[length] == '=') {
			char *start = line + length + 1;
			text_cut(start, "\r\n");
			if (*start == '\'' || *start == '"')
				start++;
			text_cut(start, "'\"");
			snprintf(value, size, "%s", start);
		}
	fclose(file);
}

void boxinfo_box_name(char *box, size_t size)
{
	char model[64];
	boxinfo_value("machinebuild", box, size);
	boxinfo_value("displaymodel", model, sizeof(model));
	for (char *c = model; *c; ++c)
		*c = (char)tolower((unsigned char)*c);
	if (!strcmp(box, "uniboxhd1") || !strcmp(box, "uniboxhd2") || !strcmp(box, "uniboxhd3"))
		snprintf(box, size, "ventonhdx");
	else if (!strcmp(box, "odinm6"))
		snprintf(box, size, "%s", model);
	else if (!strcmp(box, "inihde") && !strcmp(model, "hd-1000"))
		snprintf(box, size, "sezam-1000hd");
	else if (!strcmp(box, "ventonhdx") && !strcmp(model, "hd-5000"))
		snprintf(box, size, "sezam-5000hd");
	else if (!strcmp(box, "ventonhdx") && !strcmp(model, "premium twin"))
		snprintf(box, size, "miraclebox-twin");
	else if (!strcmp(box, "xp1000") && !strcmp(model, "sf8 hd"))
		snprintf(box, size, "sf8");
	else if (!strncmp(box, "et", 2) && strcmp(box, "et8000") && strcmp(box, "et8500") &&
		strcmp(box, "et8500s") && strcmp(box, "et10000") && strlen(box) >= 3 && size > 6) {
		box[3] = '\0';  /* et4000 -> et4x00 */
		strcat(box, "x00");
	}
	else if (!strcmp(box, "odinm9"))
		snprintf(box, size, "maram9");
	else if (!strncmp(box, "sf8008m", 7))
		snprintf(box, size, "sf8008m");
	else if (!strncmp(box, "sf8008", 6))
		snprintf(box, size, "sf8008");
	else if (!strncmp(box, "ustym4kpro", 10))
		snprintf(box, size, "ustym4kpro");
	else if (!strncmp(box, "twinboxlcdci", 12))
		snprintf(box, size, "twinboxlcd");
	else if (!strcmp(box, "sfx6018"))
		snprintf(box, size, "sfx6008");
	else if (!strcmp(box, "sx888"))
		snprintf(box, size, "sx88v2");
}
