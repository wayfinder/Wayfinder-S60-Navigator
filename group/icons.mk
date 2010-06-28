ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=$(EPOCROOT)epoc32\data\z
endif

TARGETDIR=$(ZDIR)\resource\apps\wayfinder8_s60v5
AIFTARGETFILENAME=$(TARGETDIR)\wayfinder8_s60v5_aif.mif
ICONTARGETFILENAME=$(TARGETDIR)\wficons.mif

ICONDIR=..\gfx

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

RESOURCE :	
	mifconv $(AIFTARGETFILENAME) -X -Faif.iconlist
	mifconv $(ICONTARGETFILENAME) -X -Ficons.iconlist -H$(EPOCROOT)epoc32\include\wficons.mbg
		
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing