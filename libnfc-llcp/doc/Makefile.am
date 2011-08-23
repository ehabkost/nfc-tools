# $Id$

CLEANFILES =

if HAS_MSCGEN
message_sequence_charts = \
			  link-activation-as-initiator.png \
			  link-activation-as-target.png \
			  link-deactivation-as-initiator.png \
			  link-deactivation-as-target.png

all-am: $(message_sequence_charts)

.msc.png:
	$(AM_V_GEN)$(MSCGEN) -Tpng -o $@ -i $<

CLEANFILES += $(message_sequence_charts)
endif

if HAS_GRAPHVIZ
dot_graphs = \
	 data-link-connection-status.png

all-am: $(dot_graphs)

.dot.png:
	$(AM_V_GEN)$(DOT) -Tpng -o $@ $<

CLEANFILES+= $(dot_graphs)
endif

EXTRA_DIST = data-link-connection-status.dot \
	     link-activation-as-initiator.msc \
	     link-activation-as-target.msc \
	     link-deactivation-as-initiator.msc \
	     link-deactivation-as-target.msc
