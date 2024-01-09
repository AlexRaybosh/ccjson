
all: x86_64_g x86_64

#armhf_g armhf 

#armel

.PHONY: armhf_g armhf armel armel_g x86_64_g x86_64

armhf:
	$(MAKE) -f arch.mk ARCH="armhf" ROOT=$(ROOT) OPT="-O2" all

armhf_g:
	$(MAKE) -f arch.mk ARCH="armhf" ROOT=$(ROOT) SUFFIX=_g OPT="-O0 -ggdb3" all

armel:
	$(MAKE) -f arch.mk ARCH="armel" ROOT=$(ROOT) OPT="-O2" all

armel_g:
	$(MAKE) -f arch.mk ARCH="armel" ROOT=$(ROOT) SUFFIX=_g OPT="-O0 -ggdb3" all

x86_64:
	$(MAKE) -f arch.mk ARCH="x86_64" ROOT=$(ROOT) OPT="-O2" all

x86_64_g:
	$(MAKE) -f arch.mk ARCH="x86_64" ROOT=$(ROOT) SUFFIX=_g OPT="-O0 -ggdb3" all

#	$(MAKE) -f arch.mk ARCH="armel" ROOT=$(ROOT)


clean:
	$(MAKE) -f arch.mk ARCH="armhf" ROOT=$(ROOT) clean
	$(MAKE) -f arch.mk ARCH="armhf" ROOT=$(ROOT) SUFFIX=_g clean	
	$(MAKE) -f arch.mk ARCH="armel" ROOT=$(ROOT) clean
	$(MAKE) -f arch.mk ARCH="armel" ROOT=$(ROOT) SUFFIX=_g clean
	$(MAKE) -f arch.mk ARCH="x86_64" ROOT=$(ROOT) clean
	$(MAKE) -f arch.mk ARCH="x86_64" ROOT=$(ROOT) SUFFIX=_g clean	

