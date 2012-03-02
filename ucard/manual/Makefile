
all: ucard-sdk.pdf

ucard-sdk.pdf: intitial-personalisation.pdf code-generation.pdf layers.pdf applications.pdf kiosk-001.ps kiosk-002.ps

.SUFFIXES: .dot .mp .ps .pdf .png .tex .svg

MPOST?=	mpost

kiosk-001.ps kiosk-002.ps: kiosk.mp
	@echo "  MPOST    $@";
	@${MPOST} kiosk.mp > .build-log 2>&1; \
		if [ $$? -ne 0 ] || egrep -A 3 '(Error|:[0-9]+):' .build-log ; then \
			rm -f $@ .build-log; \
			exit 1; \
		fi;

.dot.png:
	@echo "  DOT      $@";
	@dot -Tpng -o"$@" $<

#.dot.pdf:
#	@echo "  DOT      $@";
#	@dot -Tpdf -o"$@" $<

.dot.ps:
	@echo "  DOT      $@";
	@dot -Tps -o"$@" $<

.dot.tex:
	@echo "  DOT2TEX  $@";
	@dot2tex --template=.dot2tex.template.tex -fpgf -c -o"$@" $<

.svg.pdf:
	@echo "  INKSCAPE $@"
	@inkscape --export-pdf="$@" "$<"

XETEX?=		xelatex
TEX_FLAGS?=	 -output-driver="xdvipdfmx -V 5" -file-line-error -halt-on-error -interaction=nonstopmode
.tex.pdf:
	@echo "  XETEX    $@"; \
	aux=`echo $< | sed 's/.tex$$/.aux/'`; \
	while [ ! -f $${aux} ] || [ "$${omd5}" != "`md5 $${aux}`" ]; do \
		if [ -f $${aux} ]; then \
			omd5=`md5 $${aux}`;\
		fi; \
		env TEXINPUTS=".${TEXINPUTS}:" ${XETEX} ${TEX_FLAGS} $< > .build-log 2>&1; \
		if [ $$? -ne 0 ] || egrep -A 3 '(Error|:[0-9]+):' .build-log ; then \
			rm -f $@ .build-log; \
			exit 1; \
		fi; \
		rm -r .build-log; \
	done || true
	@[ -f $@ ]

clean:
	rm -rf missfont.log *.aux *.log *.out *.toc intitial-personalisation.tex code-generation.tex *.ps

mrproper: clean
	rm -rf *.pdf
