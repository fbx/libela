
if HAVE_MKDOC

html-local:
	$(mkdoc) \
		--conf-file $(srcdir)/mkdoc.conf \
		--doc-path $(srcdir)/. \
		-I $(top_srcdir)/include \
		--code-path $(top_srcdir)/test \
		ela/ela.h ela/backend.h \
		ela/libevent.h ela/cf.h

clean-local:
	-rm -r html

endif
