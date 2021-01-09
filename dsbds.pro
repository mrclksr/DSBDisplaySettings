include(defs.inc)

QT	 += widgets
TEMPLATE  = subdirs
SUBDIRS	 += src lib/backend
TRANSLATIONS = locale/$${PROGRAM}_de.ts \
               locale/$${PROGRAM}_fr.ts
INSTALLS  = target dtfile locales
QMAKE_EXTRA_TARGETS += distclean cleanqm readme

target.path  = $${PREFIX}/bin
target.files = $${PROGRAM}

dtfile.path  = $${APPSDIR} 
dtfile.files = $${PROGRAM}.desktop 

readme.target = readme
readme.files = readme.mdoc
readme.commands = mandoc -mdoc readme.mdoc | perl -e \'foreach (<STDIN>) { \
		\$$_ =~ s/(.)\x08\1/\$$1/g; \$$_ =~ s/_\x08(.)/\$$1/g; \
		print \$$_ \
	}\' | sed '1,1d' > README

locales.path = $${DATADIR}

qtPrepareTool(LRELEASE, lrelease)
for(a, TRANSLATIONS) {
	cmd = $$LRELEASE $${a}
	system($$cmd)
}
locales.files += locale/*.qm
cleanqm.commands  = rm -f $${locales.files}
distclean.depends = cleanqm

