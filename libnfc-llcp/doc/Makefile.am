# $Id$

if HAS_MSCGEN

message_sequence_charts = \
			  link-activation-as-initiator.png \
			  link-activation-as-target.png

all-am: $(message_sequence_charts)

.msc.png:
	$(MSCGEN) -Tpng -o $@ -i $<

CLEANFILES = $(message_sequence_charts)
endif
