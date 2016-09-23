CC = gcc
CFLAGS = -O -Wall `imlib2-config --cflags`
EFILE = image
LIBS = `imlib2-config --libs`

DESTDIR =
INSTALL = install
INSTALLFLAGS = -c
BINDIR = /usr/bin
INSTMANFLAGS = -m 0444
MANNEWSUFFIX = _man
MANDIR = $(MANSOURCEPATH)$(MANSECT)
MANSUFFIX = $(MANSECT)$(PROJECTMANSUFFIX)
MANPATH = /usr/share/man
MANSOURCEPATH = $(MANPATH)/man
MANSECT = 1
PROJECTMANSUFFIX = x

$(EFILE):
	@echo $(CC) $(CFLAGS) -o $(EFILE) image.c $(LIBS)
	@$(CC) $(CFLAGS) -o $@ image.c $(LIBS)

install:: $(EFILE)
	$(INSTALL) $(INSTALLFLAGS) $(EFILE) $(DESTDIR)$(BINDIR)/$(EFILE)

install.man:: $(EFILE).$(MANNEWSUFFIX)
	$(INSTALL) $(INSTALLFLAGS) $(INSTMANFLAGS) $(EFILE).$(MANNEWSUFFIX) $(DESTDIR)$(MANDIR)/$(EFILE).$(MANSUFFIX)

clean:
	@echo rm $(EFILE)
	@rm $(EFILE)
