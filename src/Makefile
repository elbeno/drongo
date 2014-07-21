DIRS=	btlib \
	include \
	regexlib \
	main \

all lint clobber clean ci:
	for subdir in $(DIRS); do \
		( cd $$subdir; echo "making $@ in `pwd`"; make $@ ); \
	done
