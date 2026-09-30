#ifndef RECOVERY_I18N_H
#define RECOVERY_I18N_H

#include <libintl.h>

#ifndef LOCALEDIR
#define LOCALEDIR "/usr/share/locale"
#endif

#define _(text) gettext(text)
#define N_(text) text  /* For xgettext, translated where it is shown. */

/* The texts of ORM in the language of Enigma2, English without its locale or catalog. */
void i18n_init(void);
/* Another language for this run of ORM, e.g. "de_DE"; 0 when its locale is missing. */
int i18n_set(const char *locale);
/* The locale in use, e.g. "de_DE". */
const char *i18n_locale(void);
/* Counts the changes of the language, so texts made before can be made again. */
int i18n_generation(void);
/* 1 for a language written from the right, like Arabic. */
int i18n_rtl(void);

#endif
