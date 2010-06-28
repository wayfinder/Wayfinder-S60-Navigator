ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\shared\wayfinder8_s60v5
SPLASHTARGETFILENAME=$(TARGETDIR)\wayfinder_news.mif

ICONDIR=..\gfx

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE :
	mifconv $(SPLASHTARGETFILENAME) /c16,1 $(ICONDIR)\splash.svg	
			
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(SPLASHTARGETFILENAME)

FINAL : do_nothing