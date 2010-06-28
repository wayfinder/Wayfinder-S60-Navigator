/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioClipTable.h"
#undef USE_OLD_ENGLISH
#include "AudioClipsEnum.h"

#include<algorithm>


// These are char* so use ascii, i.e. no едц:s. This saves 20k.
// The defines may be a bit unnecessary, but cannot change everything.
#define  KWav1st "1st"
#define  KWav2nd "2nd"
#define  KWav3rd "3rd"
#define  KWav4th "4th"
#define  KWav5th "5th"
#define  KWav6th "6th"
#define  KWav7th "7th"
#define  KWav8th "8th"
#define  KWavAndThen "andthen"
#define  KWavFinalAproach "atthedestination"
#define  KWavStraight "drivestraightahead"
#define  KWavEOTurn "endofturn"
#define  KWavExit "exit"
#define  KWavExitOfRdbt "exit_ofrdbt"
#define  KWavExitInRdbt "exit_inrdbt"
#define  KWavFinish "finish"
#define  KWavHere "here"
#define  KWav100 "in100"
#define  KWav1K "in1kilometer"
#define  KWav200 "in200"
#define  KWav25 "in25"
#define  KWav2K "in2kilometers"
#define  KWav50 "in50"
#define  KWav500 "in500"
#define  KWavRoundAbout "intheroundabout"
#define  KWavKeepLeft "keepleft"
#define  KWavKeepRight "keepright"
#define  KWavMeters "metersend"
#define  KWavTake "take"
#define  KWavTurnLeft "turnleft"
#define  KWavTurnRight "turnright"
#define  KWavWrongDirection "youaredrivinginthewrongdirection"
#define  KWavOffTrack "youarenowofftrack"
#define  KWavUTurn "uturn"
#define  KWavImmediately "immediately"
#define  KWavSeSoundKamera "SeSoundKamera"
   
#define  KWavEnSoundIn100Feet "EnSoundIn100Feet"
#define  KWavEnSoundIn200Feet "EnSoundIn200Feet"
#define  KWavEnSoundIn500Feet "EnSoundIn500Feet"
#define  KWavEnSoundInAQuarterOfAMile "EnSoundInAQuarterOfAMile"
#define  KWavEnSoundInHalfAMile "EnSoundInHalfAMile"
#define  KWavEnSoundIn1Mile "EnSoundIn1Mile"
#define  KWavEnSoundIn2Miles "EnSoundIn2Miles"
#define  KWavEnSoundIn2Kilometers "EnSoundIn2Kilometers"
#define  KWavEnSoundIn1Kilometer "EnSoundIn1Kilometer"
#define  KWavEnSoundIn500Meters "EnSoundIn500Meters"
#define  KWavEnSoundIn200Meters "EnSoundIn200Meters"
#define  KWavEnSoundIn100Meters "EnSoundIn100Meters"
#define  KWavEnSoundIn50Meters "EnSoundIn50Meters"
#define  KWavEnSoundIn25Meters "EnSoundIn25Meters"
#define  KWavEnSoundIn500Yards "EnSoundIn500Yards"
#define  KWavEnSoundIn200Yards "EnSoundIn200Yards"
#define  KWavEnSoundIn100Yards "EnSoundIn100Yards"
#define  KWavEnSoundIn50Yards "EnSoundIn50Yards"
#define  KWavEnSoundIn25Yards "EnSoundIn25Yards"
#define  KWavEnSoundTheFirst "EnSoundTheFirst"
#define  KWavEnSoundTheSecond "EnSoundTheSecond"
#define  KWavEnSoundTheThird "EnSoundTheThird"
#define  KWavEnSoundTheFourth "EnSoundTheFourth"
#define  KWavEnSoundTheFifth "EnSoundTheFifth"
#define  KWavEnSoundTheSixth "EnSoundTheSixth"
#define  KWavEnSoundTheSeventh "EnSoundTheSeventh"
#define  KWavEnSoundTheEighth "EnSoundTheEighth"
#define  KWavEnSoundAndThen "EnSoundAndThen"
#define  KWavEnSoundAtTheDestination "EnSoundAtTheDestination"
#define  KWavEnSoundAtTheRoundabout "EnSoundAtTheRoundabout"
#define  KWavEnSoundDriveStraightAhead "EnSoundDriveStraightAhead"
#define  KWavEnSoundExitNoun "EnSoundExitNoun"
#define  KWavEnSoundExitVerb "EnSoundExitVerb"
#define  KWavEnSoundHere "EnSoundHere"
#define  KWavEnSoundImmediately "EnSoundImmediately"
#define  KWavEnSoundKeepLeft "EnSoundKeepLeft"
#define  KWavEnSoundKeepRight "EnSoundKeepRight"
#define  KWavEnSoundMakeAUTurn "EnSoundMakeAUTurn"
#define  KWavEnSoundPleaseTurnAround "EnSoundPleaseTurnAround"
#define  KWavEnSoundTake "EnSoundTake"
#define  KWavEnSoundTurnLeft "EnSoundTurnLeft"
#define  KWavEnSoundTurnRight "EnSoundTurnRight"
#define  KWavEnSoundYouAreNowOffTrack "EnSoundYouAreNowOffTrack"
#define  KWavEnSoundYouHaveReachedYourDestination "EnSoundYouHaveReachedYourDestination"
#define  KWavEnSoundCamera "EnSoundSpeedCamera"


#define  KWavDeSoundDieErste "DeSoundDieErste"
#define  KWavDeSoundDieZweite "DeSoundDieZweite"
#define  KWavDeSoundDieDritte "DeSoundDieDritte"
#define  KWavDeSoundDieVierte "DeSoundDieVierte"
#define  KWavDeSoundDieFunfte "DeSoundDieFunfte"
#define  KWavDeSoundDieSechte "DeSoundDieSechte"
#define  KWavDeSoundDieSibte "DeSoundDieSibte"
#define  KWavDeSoundDieAchrte "DeSoundDieAchrte"
#define  KWavDeSoundUndDann "DeSoundUndDann"
//_LIT( KWavDeSoundDasZielBefindetSichGeradeaus,
//      "DeSoundDasZielBefindetSichGeradeaus" );
#define  KWavDeSoundAmZiel "DeSoundAmZiel"
#define  KWavDeSoundGeradeausWeiterfahren "DeSoundGeradeausWeiterfahren"
#define  KWavDeSoundAusfahrt "DeSoundAusfahrt"
#define  KWavDeSoundAbfahren "DeSoundAbfahren"
#define  KWavDeSoundSieHabenDasZielErreicht "DeSoundSieHabenDasZielErreicht"
#define  KWavDeSoundHier "DeSoundHier"
#define  KWavDeSoundInFunfUndZwanzigMetern "DeSoundInFunfUndZwanzigMetern"
#define  KWavDeSoundInFunfzigMetern "DeSoundInFunfzigMetern"
#define  KWavDeSoundInEinhundertMetern "DeSoundInEinhundertMetern"
#define  KWavDeSoundInZweihundertMetern "DeSoundInZweihundertMetern"
#define  KWavDeSoundInFunfhundertMetern "DeSoundInFunfhundertMetern"
#define  KWavDeSoundInEinemKilometer "DeSoundInEinemKilometer"
#define  KWavDeSoundInZweiKilometern "DeSoundInZweiKilometern"
#define  KWavDeSoundImKreisverkehr "DeSoundImKreisverkehr"
#define  KWavDeSoundImKreisel "DeSoundImKreisel"
#define  KWavDeSoundLinksHalten "DeSoundLinksHalten"
#define  KWavDeSoundRechtsHalten "DeSoundRechtsHalten"
#define  KWavDeSoundLinksAbbiegen "DeSoundLinksAbbiegen"
#define  KWavDeSoundRechtsAbbiegen "DeSoundRechtsAbbiegen"
#define  KWavDeSoundSieFahrenJetztInDieFalsheRichtung "DeSoundSieFahrenJetztInDieFalsheRichtung"
#define  KWavDeSoundSieHabenJetztDieGeplanteRouteVerlassen "DeSoundSieHabenJetztDieGeplanteRouteVerlassen"
#define  KWavDeSoundBitteWenden "DeSoundBitteWenden"
#define  KWavDeSoundWennMoglichBitteWenden "DeSoundWennMoglichBitteWenden"
#define  KWavDeSoundSofort "DeSoundSofort"
#define  KWavDeSoundCamera "DeSoundRadarfalle"

#define  KWavItSoundLaPrima "ItSoundLaPrima"
#define  KWavItSoundLaSeconda "ItSoundLaSeconda"
#define  KWavItSoundLaTerza "ItSoundLaTerza"
#define  KWavItSoundLaQuarta "ItSoundLaQuarta"
#define  KWavItSoundLaQuinta "ItSoundLaQuinta"
#define  KWavItSoundLaSesta "ItSoundLaSesta"
#define  KWavItSoundLaSettima "ItSoundLaSettima"
#define  KWavItSoundLottava "ItSoundLottava"
#define  KWavItSoundQuindi "ItSoundQuindi"
#define  KWavItSoundADestinazione "ItSoundADestinazione"
#define  KWavItSoundProcedoDiritto "ItSoundProcedoDiritto"
#define  KWavItSoundUscita "ItSoundUscita"
#define  KWavItSoundEsci "ItSoundEsci"
#define  KWavItSoundHaiRaggiunto "ItSoundHaiRaggiunto"
#define  KWavItSoundQui "ItSoundQui"
#define  KWavItSoundTra25Metri "ItSoundTra25Metri"
#define  KWavItSoundTra50Metri "ItSoundTra50Metri"
#define  KWavItSoundTra100Metri "ItSoundTra100Metri"
#define  KWavItSoundTra200Metri "ItSoundTra200Metri"
#define  KWavItSoundTra500Metri "ItSoundTra500Metri"
#define  KWavItSoundTra1Chilometro "ItSoundTra1Chilometro"
#define  KWavItSoundTra2Chilometri "ItSoundTra2Chilometri"
#define  KWavItSoundAllaRotonda "ItSoundAllaRotonda"
#define  KWavItSoundMantieniLaSinistra "ItSoundMantieniLaSinistra"
#define  KWavItSoundMantieniLaDestra "ItSoundMantieniLaDestra"
#define  KWavItSoundSvoltaASinistra "ItSoundSvoltaASinistra"
#define  KWavItSoundSvoltaADestra "ItSoundSvoltaADestra"
#define  KWavItSoundSeiFuoriStrada "ItSoundSeiFuoriStrada"
#define  KWavItSoundStaiProcedendo "ItSoundStaiProcedendo"
#define  KWavItSoundQuandoPossibile "ItSoundQuandoPossibile"
#define  KWavItSoundImmediatamente "ItSoundImmediatamente"
#define  KWavItSoundPrendi "ItSoundPrendi"
#define  KWavItSoundEsciDallAutostrada "ItSoundEsciDallAutostrada"
#define  KWavItSoundEseguiUnInversioneAU "ItSoundEseguiUnInversioneAU"
#define  KWavItSoundCamera "ItSoundFotocamere"

#define  KWavFrSoundLaPremiere "FrSoundLaPremiere"
#define  KWavFrSoundLaDeuxieme "FrSoundLaDeuxieme"
#define  KWavFrSoundLaTroisieme "FrSoundLaTroisieme"
#define  KWavFrSoundLaQuatrieme "FrSoundLaQuatrieme"
#define  KWavFrSoundLaCinquieme "FrSoundLaCinquieme"
#define  KWavFrSoundLaSixieme "FrSoundLaSixieme"
#define  KWavFrSoundLaSeptieme "FrSoundLaSeptieme"
#define  KWavFrSoundLaHuitieme "FrSoundLaHuitieme"
#define  KWavFrSoundImmediatement "FrSoundImmediatement"
#define  KWavFrSoundIci "FrSoundIci"
#define  KWavFrSoundA25Metres "FrSoundA25Metres"
#define  KWavFrSoundA50Metres "FrSoundA50Metres"
#define  KWavFrSoundA100Metres "FrSoundA100Metres"
#define  KWavFrSoundA200Metres "FrSoundA200Metres"
#define  KWavFrSoundA500Metres "FrSoundA500Metres"
#define  KWavFrSoundA1Kilometre "FrSoundA1Kilometre"
#define  KWavFrSoundA2Kilometres "FrSoundA2Kilometres"
#define  KWavFrSoundADestination "FrSoundADestination"
#define  KWavFrSoundADroite "FrSoundADroite"
#define  KWavFrSoundAGauche "FrSoundAGauche"
#define  KWavFrSoundAPresentVousEtes "FrSoundAPresentVousEtes"
#define  KWavFrSoundAuRondpoint "FrSoundAuRondpoint"
#define  KWavFrSoundConduisezToutDroit "FrSoundConduisezToutDroit"
#define  KWavFrSoundContinuez "FrSoundContinuez"
#define  KWavFrSoundDesQuePossible "FrSoundDesQuePossible"
#define  KWavFrSoundFaitesDemitour "FrSoundFaitesDemitour"
#define  KWavFrSoundPrenez "FrSoundPrenez"
#define  KWavFrSoundQuittezLautoroute "FrSoundQuittezLautoroute"
#define  KWavFrSoundSortez "FrSoundSortez"
#define  KWavFrSoundSortie "FrSoundSortie"
#define  KWavFrSoundTournez "FrSoundTournez"
#define  KWavFrSoundVousEtesArrive "FrSoundVousEtesArrive"
#define  KWavFrSoundPuis "FrSoundPuis"
#define  KWavFrSoundCamera "FrSoundRadar"

#define  KWavEsSoundAqui "EsSoundAqui"
#define  KWavEsSoundCambiaDeSentido "EsSoundCambiaDeSentido"
#define  KWavEsSoundConduceTodoRecto "EsSoundConduceTodoRecto"
#define  KWavEsSoundCuandoPuedas "EsSoundCuandoPuedas"
#define  KWavEsSoundElDestino "EsSoundElDestino"
#define  KWavEsSoundEn100Metros "EsSoundEn100Metros"
#define  KWavEsSoundEn1Kilometro "EsSoundEn1Kilometro"
#define  KWavEsSoundEn200Metros "EsSoundEn200Metros"
#define  KWavEsSoundEn25Metros "EsSoundEn25Metros"
#define  KWavEsSoundEn2Kilometros "EsSoundEn2Kilometros"
#define  KWavEsSoundEn500Metros "EsSoundEn500Metros"
#define  KWavEsSoundEn50Metros "EsSoundEn50Metros"
#define  KWavEsSoundEnElDestino "EsSoundEnElDestino"
#define  KWavEsSoundEnLaRotonda "EsSoundEnLaRotonda"
#define  KWavEsSoundEstasConduciendo "EsSoundEstasConduciendo"
#define  KWavEsSoundEstasFueraDeLaRuta "EsSoundEstasFueraDeLaRuta"
#define  KWavEsSoundGiraALaDerecha "EsSoundGiraALaDerecha"
#define  KWavEsSoundGiraALaIzquierda "EsSoundGiraALaIzquierda"
#define  KWavEsSoundHasLlegadoATuDestino "EsSoundHasLlegadoATuDestino"
#define  KWavEsSoundInmeditamente "EsSoundInmeditamente"
#define  KWavEsSoundLaCuarta "EsSoundLaCuarta"
#define  KWavEsSoundLaOctava "EsSoundLaOctava"
#define  KWavEsSoundLaPrimera "EsSoundLaPrimera"
#define  KWavEsSoundLaQuinta "EsSoundLaQuinta"
#define  KWavEsSoundLaSegunda "EsSoundLaSegunda"
#define  KWavEsSoundLaSeptima "EsSoundLaSeptima"
#define  KWavEsSoundLaSexta "EsSoundLaSexta"
#define  KWavEsSoundLaTercera "EsSoundLaTercera"
#define  KWavEsSoundMantenteALaDerecha "EsSoundMantenteALaDerecha"
#define  KWavEsSoundMantenteALaIzquierda "EsSoundMantenteALaIzquierda"
#define  KWavEsSoundSalida "EsSoundSalida"
#define  KWavEsSoundSal "EsSoundSal"
#define  KWavEsSoundSalDeLaAutopista "EsSoundSalDeLaAutopista"
#define  KWavEsSoundToma "EsSoundToma"
#define  KWavEsSoundYDespues "EsSoundYDespues"
#define  KWavEsSoundCamera "EsSoundCamaraDeTrafico"

         /* dutch */
#define  KWavNlSoundAfrit "NlSoundAfrit"
#define  KWavNlSoundBijDeBestemming "NlSoundBijDeBestemming"
#define  KWavNlSoundDeEerste "NlSoundDeEerste"
#define  KWavNlSoundDeTweede "NlSoundDeTweede"
#define  KWavNlSoundDeDerde "NlSoundDeDerde"
#define  KWavNlSoundDeVierde "NlSoundDeVierde"
#define  KWavNlSoundDeVijfde "NlSoundDeVijfde"
#define  KWavNlSoundDeZesde "NlSoundDeZesde"
#define  KWavNlSoundDeZevende "NlSoundDeZevende"
#define  KWavNlSoundDeAchtste "NlSoundDeAchtste"
#define  KWavNlSoundEnDan "NlSoundEnDan"
#define  KWavNlSoundHier "NlSoundHier"
#define  KWavNlSoundKeerOmIndienMogelijk "NlSoundKeerOmIndienMogelijk"
#define  KWavNlSoundLinksAfslaan "NlSoundLinksAfslaan"
#define  KWavNlSoundLinksHouden "NlSoundLinksHouden"
#define  KWavNlSoundLinks "NlSoundLinks"
#define  KWavNlSoundNa100Meter "NlSoundNa100Meter"
#define  KWavNlSoundNa1Kilometer "NlSoundNa1Kilometer"
#define  KWavNlSoundNa200Meter "NlSoundNa200Meter"
#define  KWavNlSoundNa25Meter "NlSoundNa25Meter"
#define  KWavNlSoundNa2Kilometer "NlSoundNa2Kilometer"
#define  KWavNlSoundNa500Meter "NlSoundNa500Meter"
#define  KWavNlSoundNa50Meter "NlSoundNa50Meter"
#define  KWavNlSoundNaarLinks "NlSoundNaarLinks"
#define  KWavNlSoundNaarRechts "NlSoundNaarRechts"
#define  KWavNlSoundOmkeren "NlSoundOmkeren"
#define  KWavNlSoundOnmiddellijk "NlSoundOnmiddellijk"
#define  KWavNlSoundOpDeRotonde "NlSoundOpDeRotonde"
#define  KWavNlSoundRechtdoorRijden "NlSoundRechtdoorRijden"
#define  KWavNlSoundRechtsAfslaan "NlSoundRechtsAfslaan"
#define  KWavNlSoundRechtsHouden "NlSoundRechtsHouden"
#define  KWavNlSoundRechts "NlSoundRechts"
#define  KWavNlSoundSnelwegVerlaten "NlSoundSnelwegVerlaten"
#define  KWavNlSoundUBevindtZichNuBuitenDeRoute "NlSoundUBevindtZichNuBuitenDeRoute"
#define  KWavNlSoundUHebtUwBestemmingBereikt "NlSoundUHebtUwBestemmingBereikt"
#define  KWavNlSoundURijdtInDeVerkeerdeRichting "NlSoundURijdtInDeVerkeerdeRichting"
#define  KWavNlSoundCamera "NlSoundCamera"

#define  KWavFiSound25Metrin "FiSound25Metrin"
#define  KWavFiSound50Metrin "FiSound50Metrin"
#define  KWavFiSound100Metrin "FiSound100Metrin"
#define  KWavFiSound200Metrin "FiSound200Metrin"
#define  KWavFiSound500Metrin "FiSound500Metrin"
#define  KWavFiSound1Kilometrin "FiSound1Kilometrin"
#define  KWavFiSound2Kilometrin "FiSound2Kilometrin"
#define  KWavFiSoundEnsimmainen "FiSoundEnsimmainen"
#define  KWavFiSoundToinen "FiSoundToinen"
#define  KWavFiSoundKolmas "FiSoundKolmas"
#define  KWavFiSoundNeljas "FiSoundNeljas"
#define  KWavFiSoundViides "FiSoundViides"
#define  KWavFiSoundKuudes "FiSoundKuudes"
#define  KWavFiSoundSeitsemas "FiSoundSeitsemas"
#define  KWavFiSoundKahdeksas "FiSoundKahdeksas"
#define  KWavFiSoundEtSeuraaAnnettuaReittia "FiSoundEtSeuraaAnnettuaReittia"
#define  KWavFiSoundJaSitten "FiSoundJaSitten"
#define  KWavFiSoundKaannyOikealle "FiSoundKaannyOikealle"
#define  KWavFiSoundKaannyVasemmalle "FiSoundKaannyVasemmalle"
#define  KWavFiSoundJalkeen "FiSoundJalkeen"
#define  KWavFiSoundPaassa "FiSoundPaassa"
#define  KWavFiSoundLiikenneympyrasta "FiSoundLiikenneympyrasta"
#define  KWavFiSoundOletTullutPerille "FiSoundOletTullutPerille"
#define  KWavFiSoundPoistu "FiSoundPoistu"
#define  KWavFiSoundPysyOikealla "FiSoundPysyOikealla"
#define  KWavFiSoundPysyVasemmalla "FiSoundPysyVasemmalla"
#define  KWavFiSoundTassa "FiSoundTassa"
#define  KWavFiSoundTasta "FiSoundTasta"
#define  KWavFiSoundTeeUKaannosHeti "FiSoundTeeUKaannosHeti"
#define  KWavFiSoundUloskaynti "FiSoundUloskaynti"
#define  KWavFiSoundValitseLiikenneympyrasta "FiSoundValitseLiikenneympyrasta"
#define  KWavFiSoundValittomasti "FiSoundValittomasti"
#define  KWavFiSoundKaanny "FiSoundKaanny"
#define  KWavFiSoundYmpari "FiSoundYmpari"
#define  KWavFiSoundMaaranpaasiOn "FiSoundMaaranpaasiOn"
#define  KWavFiSoundSuoraanEteenpain "FiSoundSuoraanEteenpain"
#define  KWavFiSoundAjaLiikenneymparastaSouraanEteenpain "FiSoundAjaLiikenneymparastaSouraanEteenpain"
#define  KWavFiSoundOikealle "FiSoundOikealle"
#define  KWavFiSoundVasemmalle "FiSoundVasemmalle"
#define  KWavFiSoundCamera "FiSoundKamera"

#define  KWavNoSound25 "NoSound25"
#define  KWavNoSound50 "NoSound50"
#define  KWavNoSound100 "NoSound100"
#define  KWavNoSound200 "NoSound200"
#define  KWavNoSound500 "NoSound500"
#define  KWavNoSound1 "NoSound1"
#define  KWavNoSound2 "NoSound2"
#define  KWavNoSoundForste "NoSoundForste"
#define  KWavNoSoundAndre "NoSoundAndre"
#define  KWavNoSoundTredje "NoSoundTredje"
#define  KWavNoSoundFjerde "NoSoundFjerde"
#define  KWavNoSoundFemte "NoSoundFemte"
#define  KWavNoSoundSjette "NoSoundSjette"
#define  KWavNoSoundSjuende "NoSoundSjuende"
#define  KWavNoSoundAttende "NoSoundAttende"
#define  KWavNoSoundAvkjoring "NoSoundAvkjoring"
#define  KWavNoSoundDestinasjonenErRettFrem "NoSoundDestinasjonenErRettFrem"
#define  KWavNoSoundDuErFremmeVedDestinasjonen "NoSoundDuErFremmeVedDestinasjonen"
#define  KWavNoSoundDuErUtenforRuten "NoSoundDuErUtenforRuten"
#define  KWavNoSoundForlatHovedveien "NoSoundForlatHovedveien"
#define  KWavNoSoundGjorEnUSvingNarDetBlirMulig "NoSoundGjorEnUSvingNarDetBlirMulig"
#define  KWavNoSoundHer "NoSoundHer"
#define  KWavNoSoundHold "NoSoundHold"
#define  KWavNoSoundIEndenAvGaten "NoSoundIEndenAvGaten"
#define  KWavNoSoundIRundkjoringen "NoSoundIRundkjoringen"
#define  KWavNoSoundKilometer "NoSoundKilometer"
#define  KWavNoSoundKjorAv "NoSoundKjorAv"
#define  KWavNoSoundKjorHeleVeienRundt "NoSoundKjorHeleVeienRundt"
#define  KWavNoSoundKjorRettFrem "NoSoundKjorRettFrem"
#define  KWavNoSoundKjorUt "NoSoundKjorUt"
#define  KWavNoSoundMeter "NoSoundMeter"
#define  KWavNoSoundMiles "NoSoundMiles"
#define  KWavNoSoundOgSa "NoSoundOgSa"
#define  KWavNoSoundOm "NoSoundOm"
#define  KWavNoSoundSving "NoSoundSving"
#define  KWavNoSoundTa "NoSoundTa"
#define  KWavNoSoundTilHoyre "NoSoundTilHoyre"
#define  KWavNoSoundTilVenstre "NoSoundTilVenstre"
#define  KWavNoSoundUmiddelbart "NoSoundUmiddelbart"
#define  KWavNoSoundVedDestinasjonen "NoSoundVedDestinasjonen"
#define  KWavNoSoundVennligstSnu "NoSoundVennligstSnu"
#define  KWavNoSoundVenstre "NoSoundVenstre"
#define  KWavNoSoundYards "NoSoundYards"
#define  KWavNoSoundCamera "NoSoundKamera"

#define  KWavUsSoundIn100Feet "UsSoundIn100Feet"
#define  KWavUsSoundIn200Feet "UsSoundIn200Feet"
#define  KWavUsSoundIn500Feet "UsSoundIn500Feet"
#define  KWavUsSoundInAQuarterOfAMile "UsSoundInAQuarterOfAMile"
#define  KWavUsSoundInHalfAMile "UsSoundInHalfAMile"
#define  KWavUsSoundIn1Mile "UsSoundIn1Mile"
#define  KWavUsSoundIn2Miles "UsSoundIn2Miles"
#define  KWavUsSoundIn2Kilometers "UsSoundIn2Kilometers"
#define  KWavUsSoundIn1Kilometer "UsSoundIn1Kilometer"
#define  KWavUsSoundIn500Meters "UsSoundIn500Meters"
#define  KWavUsSoundIn200Meters "UsSoundIn200Meters"
#define  KWavUsSoundIn100Meters "UsSoundIn100Meters"
#define  KWavUsSoundIn50Meters "UsSoundIn50Meters"
#define  KWavUsSoundIn25Meters "UsSoundIn25Meters"
#define  KWavUsSoundIn500Yards "UsSoundIn500Yards"
#define  KWavUsSoundIn200Yards "UsSoundIn200Yards"
#define  KWavUsSoundIn100Yards "UsSoundIn100Yards"
#define  KWavUsSoundIn50Yards "UsSoundIn50Yards"
#define  KWavUsSoundIn25Yards "UsSoundIn25Yards"
#define  KWavUsSoundTheFirst "UsSoundTheFirst"
#define  KWavUsSoundTheSecond "UsSoundTheSecond"
#define  KWavUsSoundTheThird "UsSoundTheThird"
#define  KWavUsSoundTheFourth "UsSoundTheFourth"
#define  KWavUsSoundTheFifth "UsSoundTheFifth"
#define  KWavUsSoundTheSixth "UsSoundTheSixth"
#define  KWavUsSoundTheSeventh "UsSoundTheSeventh"
#define  KWavUsSoundTheEighth "UsSoundTheEighth"
#define  KWavUsSoundAndThen "UsSoundAndThen"
#define  KWavUsSoundAtTheDestination "UsSoundAtTheDestination"
#define  KWavUsSoundAtTheRoundabout "UsSoundAtTheRoundabout"
#define  KWavUsSoundDriveStraightAhead "UsSoundDriveStraightAhead"
#define  KWavUsSoundExitNoun "UsSoundExitNoun"
#define  KWavUsSoundExitVerb "UsSoundExitVerb"
#define  KWavUsSoundHere "UsSoundHere"
#define  KWavUsSoundImmediately "UsSoundImmediately"
#define  KWavUsSoundKeepLeft "UsSoundKeepLeft"
#define  KWavUsSoundKeepRight "UsSoundKeepRight"
#define  KWavUsSoundMakeAUTurn "UsSoundMakeAUTurn"
#define  KWavUsSoundPleaseTurnAround "UsSoundPleaseTurnAround"
#define  KWavUsSoundTake "UsSoundTake"
#define  KWavUsSoundTurnLeft "UsSoundTurnLeft"
#define  KWavUsSoundTurnRight "UsSoundTurnRight"
#define  KWavUsSoundYouAreNowOffTrack "UsSoundYouAreNowOffTrack"
#define  KWavUsSoundYouHaveReachedYourDestination "UsSoundYouHaveReachedYourDestination"
#define  KWavUsSoundCamera "UsSoundSpeedCamera"

#define  KWavPtSound100Metros "PtSound100Metros"
#define  KWavPtSound1Quilometro "PtSound1Quilometro"
#define  KWavPtSound200Metros "PtSound200Metros"
#define  KWavPtSound25Metros "PtSound25Metros"
#define  KWavPtSound2Quilometros "PtSound2Quilometros"
#define  KWavPtSound500Metros "PtSound500Metros"
#define  KWavPtSound50Metros "PtSound50Metros"
#define  KWavPtSoundAqui "PtSoundAqui"
#define  KWavPtSoundA "PtSoundA"
#define  KWavPtSoundDaquiA "PtSoundDaquiA"
#define  KWavPtSoundEASeguir "PtSoundEASeguir"
#define  KWavPtSoundFacaInversaoDeMarcha "PtSoundFacaInversaoDeMarcha"
#define  KWavPtSoundImediatamente "PtSoundImediatamente"
#define  KWavPtSoundMantenhaSeADireita "PtSoundMantenhaSeADireita"
#define  KWavPtSoundMantenhaSeAEsquerda "PtSoundMantenhaSeAEsquerda"
#define  KWavPtSoundNaOitava "PtSoundNaOitava"
#define  KWavPtSoundNaPrimeira "PtSoundNaPrimeira"
#define  KWavPtSoundNaQuarta "PtSoundNaQuarta"
#define  KWavPtSoundNaQuinta "PtSoundNaQuinta"
#define  KWavPtSoundNaRotunda "PtSoundNaRotunda"
#define  KWavPtSoundNaSegunda "PtSoundNaSegunda"
#define  KWavPtSoundNaSetima "PtSoundNaSetima"
#define  KWavPtSoundNaSexta "PtSoundNaSexta"
#define  KWavPtSoundNaTerceira "PtSoundNaTerceira"
#define  KWavPtSoundNoDestino "PtSoundNoDestino"
#define  KWavPtSoundQuandoPossivel "PtSoundQuandoPossivel"
#define  KWavPtSoundSaia "PtSoundSaia"
#define  KWavPtSoundSaida "PtSoundSaida"
#define  KWavPtSoundSigaEmFrente "PtSoundSigaEmFrente"
#define  KWavPtSoundVireADireita "PtSoundVireADireita"
#define  KWavPtSoundVireAEsquerda "PtSoundVireAEsquerda"
#define  KWavPtSoundVire "PtSoundVire"
#define  KWavPtSoundVoceChegouAoSeuDestino "PtSoundVoceChegouAoSeuDestino"
#define  KWavPtSoundVoceEstaForaDeRota "PtSoundVoceEstaForaDeRota"
#define  KWavPtSoundCamera "PtSoundCamera"

#define  KWavHuSound100 "HuSound100"
#define  KWavHuSound1 "HuSound1"
#define  KWavHuSound200 "HuSound200"
#define  KWavHuSound25 "HuSound25"
#define  KWavHuSound2 "HuSound2"
#define  KWavHuSound500 "HuSound500"
#define  KWavHuSound50 "HuSound50"
#define  KWavHuSoundAzElso "HuSoundAzElso"
#define  KWavHuSoundAMasodik "HuSoundAMasodik"
#define  KWavHuSoundAHarmadik "HuSoundAHarmadik"
#define  KWavHuSoundANegyedik "HuSoundANegyedik"
#define  KWavHuSoundAzOtodik "HuSoundAzOtodik"
#define  KWavHuSoundAHatodik "HuSoundAHatodik"
#define  KWavHuSoundAHetedik "HuSoundAHetedik"
#define  KWavHuSoundANyolcadik "HuSoundANyolcadik"
#define  KWavHuSoundAKijaratig "HuSoundAKijaratig"
#define  KWavHuSoundAKorforgalomnal "HuSoundAKorforgalomnal"
#define  KWavHuSoundAKorforgalomban "HuSoundAKorforgalomban"
#define  KWavHuSoundACelpontElottedVan "HuSoundACelpontElottedVan"
#define  KWavHuSoundACelpontig "HuSoundACelpontig"
#define  KWavHuSoundBalra "HuSoundBalra"
#define  KWavHuSoundEgyenesen "HuSoundEgyenesen"
#define  KWavHuSoundElertedACelpontot "HuSoundElertedACelpontot"
#define  KWavHuSoundFel "HuSoundFel"
#define  KWavHuSoundForduljVisszaAmintLehet "HuSoundForduljVisszaAmintLehet"
#define  KWavHuSoundForduljVissza "HuSoundForduljVissza"
#define  KWavHuSoundFordulj "HuSoundFordulj"
#define  KWavHuSoundHajtsKi "HuSoundHajtsKi"
#define  KWavHuSoundItt "HuSoundItt"
#define  KWavHuSoundJardot "HuSoundJardot"
#define  KWavHuSoundJobbra "HuSoundJobbra"
#define  KWavHuSoundKijaraton "HuSoundKijaraton"
#define  KWavHuSoundKilometert "HuSoundKilometert"
#define  KWavHuSoundLabat "HuSoundLabat"
#define  KWavHuSoundMajdUtana "HuSoundMajdUtana"
#define  KWavHuSoundMenjTovabbEgyenesen1 "HuSoundMenjTovabbEgyenesen1"
#define  KWavHuSoundMenjTovabbEgyenesen2 "HuSoundMenjTovabbEgyenesen2"
#define  KWavHuSoundMenj "HuSoundMenj"
#define  KWavHuSoundMerfoldet "HuSoundMerfoldet"
#define  KWavHuSoundMetert "HuSoundMetert"
#define  KWavHuSoundMostLetertelAzUtvonalrol "HuSoundMostLetertelAzUtvonalrol"
#define  KWavHuSoundMost "HuSoundMost"
#define  KWavHuSoundNegyed "HuSoundNegyed"
#define  KWavHuSoundNyolcad "HuSoundNyolcad"
#define  KWavHuSoundRosszIranybaMesz "HuSoundRosszIranybaMesz"
#define  KWavHuSoundTarts "HuSoundTarts"
#define  KWavHuSoundCamera "HuSoundTrafipax"

#define  KWavPlSoundWPierwszy "PlSoundWPierwszy"
#define  KWavPlSoundWDrugi "PlSoundWDrugi"
#define  KWavPlSoundWTrzeci "PlSoundWTrzeci"
#define  KWavPlSoundWCzwarty "PlSoundWCzwarty"
#define  KWavPlSoundWPiaty "PlSoundWPiaty"
#define  KWavPlSoundWSzusty "PlSoundWSzusty"
#define  KWavPlSoundWSiodmy "PlSoundWSiodmy"
#define  KWavPlSoundWOsmy "PlSoundWOsmy"
#define  KWavPlSoundTutaj "PlSoundTutaj"
#define  KWavPlSoundNatychmiast "PlSoundNatychmiast"
#define  KWavPlSoundZa25Metrow "PlSoundZa25Metrow"
#define  KWavPlSoundZa50Metrow "PlSoundZa50Metrow"
#define  KWavPlSoundZa100Metrow "PlSoundZa100Metrow"
#define  KWavPlSoundZa200Metrow "PlSoundZa200Metrow"
#define  KWavPlSoundZa500Metrow "PlSoundZa500Metrow"
#define  KWavPlSoundZa25Jardow "PlSoundZa25Jardow"
#define  KWavPlSoundZa50Jardow "PlSoundZa50Jardow"
#define  KWavPlSoundZa100Jardow "PlSoundZa100Jardow"
#define  KWavPlSoundZa200Jardow "PlSoundZa200Jardow"
#define  KWavPlSoundZa500Jardow "PlSoundZa500Jardow"
#define  KWavPlSoundZa25Stop "PlSoundZa25Stop"
#define  KWavPlSoundZa50Stop "PlSoundZa50Stop"
#define  KWavPlSoundZa100Stop "PlSoundZa100Stop"
#define  KWavPlSoundZa200Stop "PlSoundZa200Stop"
#define  KWavPlSoundZa500Stop "PlSoundZa500Stop"
#define  KWavPlSoundZa1Kilometr "PlSoundZa1Kilometr"
#define  KWavPlSoundZa2Kilometry "PlSoundZa2Kilometry"
#define  KWavPlSoundZaPuMili "PlSoundZaPuMili"
#define  KWavPlSoundZaJednotwartoMili "PlSoundZaJednotwartoMili"
#define  KWavPlSoundZaJednoossmoMili "PlSoundZaJednoossmoMili"
#define  KWavPlSoundZa1Mile "PlSoundZa1Mile"
#define  KWavPlSoundZa2Mile "PlSoundZa2Mile"
#define  KWavPlSoundAPotem "PlSoundAPotem"
#define  KWavPlSoundDojechales "PlSoundDojechales"
#define  KWavPlSoundJedz "PlSoundJedz"
#define  KWavPlSoundJedzProsto "PlSoundJedzProsto"
#define  KWavPlSoundKamera "PlSoundKamera"
#define  KWavPlSoundNaRondzie "PlSoundNaRondzie"
#define  KWavPlSoundOstrzezenie "PlSoundOstrzezenie"
#define  KWavPlSoundProszeZawrocic "PlSoundProszeZawrocic"
#define  KWavPlSoundPunkt "PlSoundPunkt"
#define  KWavPlSoundSkrecWLewo "PlSoundSkrecWLewo"
#define  KWavPlSoundSkrecWPrawo "PlSoundSkrecWPrawo"
#define  KWavPlSoundTeraz "PlSoundTeraz"
#define  KWavPlSoundTrzymajSieLewej "PlSoundTrzymajSieLewej"
#define  KWavPlSoundTrzymajSiePrawej "PlSoundTrzymajSiePrawej"
#define  KWavPlSoundWlasnie "PlSoundWlasnie"
#define  KWavPlSoundWMiejscuPrzeznaczenia "PlSoundWMiejscuPrzeznaczenia"
#define  KWavPlSoundZawrocGdyBedzieToMozliwe "PlSoundZawrocGdyBedzieToMozliwe"
#define  KWavPlSoundZjazd "PlSoundZjazd"
#define  KWavPlSoundZjedz "PlSoundZjedz"
#define  KWavPlSoundZjedzZAutostrade "PlSoundZjedzZAutostrade"
#define  KWavPlSoundCamera "PlSoundKamera"

#define  KWavCsSoundPrvnim "CsSoundPrvnim"
#define  KWavCsSoundDruhym "CsSoundDruhym"
#define  KWavCsSoundTretim "CsSoundTretim"
#define  KWavCsSoundCtvrtym "CsSoundCtvrtym"
#define  KWavCsSoundPatym "CsSoundPatym"
#define  KWavCsSoundSestym "CsSoundSestym"
#define  KWavCsSoundSedmym "CsSoundSedmym"
#define  KWavCsSoundOsmym "CsSoundOsmym"
#define  KWavCsSoundZde "CsSoundZde"
#define  KWavCsSoundIhned "CsSoundIhned"
#define  KWavCsSoundPo25Metrech "CsSoundPo25Metrech"
#define  KWavCsSoundPo50Metrech "CsSoundPo50Metrech"
#define  KWavCsSoundPo100Metrech "CsSoundPo100Metrech"
#define  KWavCsSoundPo200Metrech "CsSoundPo200Metrech"
#define  KWavCsSoundPo500Metrech "CsSoundPo500Metrech"
#define  KWavCsSoundPo25Yardech "CsSoundPo25Yardech"
#define  KWavCsSoundPo50Yardech "CsSoundPo50Yardech"
#define  KWavCsSoundPo100Yardech "CsSoundPo100Yardech"
#define  KWavCsSoundPo200Yardech "CsSoundPo200Yardech"
#define  KWavCsSoundPo500Yardech "CsSoundPo500Yardech"
#define  KWavCsSoundPo25Stopach "CsSoundPo25Stopach"
#define  KWavCsSoundPo50Stopach "CsSoundPo50Stopach"
#define  KWavCsSoundPo100Stopach "CsSoundPo100Stopach"
#define  KWavCsSoundPo200Stopach "CsSoundPo200Stopach"
#define  KWavCsSoundPo500Stopach "CsSoundPo500Stopach"
#define  KWavCsSoundPo1Kilometru "CsSoundPo1Kilometru"
#define  KWavCsSoundPo2Kilometrech "CsSoundPo2Kilometrech"
#define  KWavCsSoundPoPulMile "CsSoundPoPulMile"
#define  KWavCsSoundPoCtvrtMile "CsSoundPoCtvrtMile"
#define  KWavCsSoundPoOsmineMile "CsSoundPoOsmineMile"
#define  KWavCsSoundPo1Mili "CsSoundPo1Mili"
#define  KWavCsSoundPo2Milich "CsSoundPo2Milich"
#define  KWavCsSoundAPotom "CsSoundAPotom"
#define  KWavCsSoundCilJePredVami "CsSoundCilJePredVami"
#define  KWavCsSoundDrzteSeVlevo "CsSoundDrzteSeVlevo"
#define  KWavCsSoundDrzteSeVpravo "CsSoundDrzteSeVpravo"
#define  KWavCsSoundJedte "CsSoundJedte"
#define  KWavCsSoundJedtePrimo "CsSoundJedtePrimo"
#define  KWavCsSoundJsteVCili "CsSoundJsteVCili"
#define  KWavCsSoundNaKruhovemObjezdu "CsSoundNaKruhovemObjezdu"
#define  KWavCsSoundNebezpeci "CsSoundNebezpeci"
#define  KWavCsSoundNyniJedeteNespravnymSmerem "CsSoundNyniJedeteNespravnymSmerem"
#define  KWavCsSoundNyniJsteMimoTrasu "CsSoundNyniJsteMimoTrasu"
#define  KWavCsSoundOdbocteVlevo "CsSoundOdbocteVlevo"
#define  KWavCsSoundOdbocteVpravo "CsSoundOdbocteVpravo"
#define  KWavCsSoundOpustteDalnici "CsSoundOpustteDalnici"
#define  KWavCsSoundOtocteSeAzJeToMozne "CsSoundOtocteSeAzJeToMozne"
#define  KWavCsSoundProsimOtocteSe "CsSoundProsimOtocteSe"
#define  KWavCsSoundRychlostniKamera "CsSoundRychlostniKamera"
#define  KWavCsSoundVCili "CsSoundVCili"
#define  KWavCsSoundVyjedte "CsSoundVyjedte"
#define  KWavCsSoundVyjezdem "CsSoundVyjezdem"
#define  KWavCsSoundCamera "CsSoundRychlostniKamera"

#define  KWavSlSoundPrvi "SlSoundPrvi"
#define  KWavSlSoundDrugi "SlSoundDrugi"
#define  KWavSlSoundTretji "SlSoundTretji"
#define  KWavSlSoundCetrti "SlSoundCetrti"
#define  KWavSlSoundPeti "SlSoundPeti"
#define  KWavSlSoundSesti "SlSoundSesti"
#define  KWavSlSoundSedmi "SlSoundSedmi"
#define  KWavSlSoundOsmi "SlSoundOsmi"
#define  KWavSlSoundIzhod "SlSoundIzhod"
#define  KWavSlSoundCez25Metrov "SlSoundCez25Metrov"
#define  KWavSlSoundCez50Metrov "SlSoundCez50Metrov"
#define  KWavSlSoundCez100Metrov "SlSoundCez100Metrov"
#define  KWavSlSoundCez200Metrov "SlSoundCez200Metrov"
#define  KWavSlSoundCez500Metrov "SlSoundCez500Metrov"
#define  KWavSlSoundCez1Kilometer "SlSoundCez1Kilometer"
#define  KWavSlSoundCez2Kilometra "SlSoundCez2Kilometra"
#define  KWavSlSoundInPotem "SlSoundInPotem"
#define  KWavSlSoundIzberite "SlSoundIzberite"
#define  KWavSlSoundKameriZaNadzorHitrosti "SlSoundKameriZaNadzorHitrosti"
#define  KWavSlSoundKoBoMogoceNareditePolkrozniZavoj "SlSoundKoBoMogoceNareditePolkrozniZavoj"
#define  KWavSlSoundNaCilju "SlSoundNaCilju"
#define  KWavSlSoundNaKroznemKriziscu "SlSoundNaKroznemKriziscu"
#define  KWavSlSoundNemudoma "SlSoundNemudoma"
#define  KWavSlSoundOpozorilo "SlSoundOpozorilo"
#define  KWavSlSoundOstaniteNaDesnemPasu "SlSoundOstaniteNaDesnemPasu"
#define  KWavSlSoundOstaniteNaLevemPasu "SlSoundOstaniteNaLevemPasu"
#define  KWavSlSoundPrisliSteNaSvojCilj "SlSoundPrisliSteNaSvojCilj"
#define  KWavSlSoundProsimoObrnite "SlSoundProsimoObrnite"
#define  KWavSlSoundTukaj "SlSoundTukaj"
#define  KWavSlSoundVasCiljJeNaravnost "SlSoundVasCiljJeNaravnost"
#define  KWavSlSoundZapeljiteIzAvtoceste "SlSoundZapeljiteIzAvtoceste"
#define  KWavSlSoundZapeljiteNaravnost "SlSoundZapeljiteNaravnost"
#define  KWavSlSoundZavijteDesno "SlSoundZavijteDesno"
#define  KWavSlSoundZavijteLevo "SlSoundZavijteLevo"
#define  KWavSlSoundZavijte "SlSoundZavijte"
#define  KWavSlSoundZdajSteZasliSPoti "SlSoundZdajSteZasliSPoti"


#ifdef __WINS__
#define XX(x,y) int_to_clipname_t(isab::AudioClipsEnum::x,y)
#else
#define XX(x,y) { isab::AudioClipsEnum::x,y }
#endif

const AudioClipTableTableDriven::int_to_clipname_t
OldAudioClipTable::m_staticClipTable1 [] =
{ XX(SoundNewCrossing,              KWavEOTurn ),
  // The clips must be sorted in AudioClipEnum-order
  XX(SvSoundSvangVanster,           KWavTurnLeft ),
  XX(SvSoundSvangHoger,             KWavTurnRight ),
  XX(SvSoundHaer,                   KWavHere ),
  XX(SvSoundOmTjugofemMeter,        KWav25 ),
  XX(SvSoundOmFemtioMeter,          KWav50 ),
  XX(SvSoundOmEttHundraMeter,       KWav100 ),
  XX(SvSoundOmTvaHundraMeter,       KWav200 ),
  XX(SvSoundOmFemHundraMeter,       KWav500 ),
  XX(SvSoundOmEnKilometer,          KWav1K ),
  XX(SvSoundOmTvaKilometer,         KWav2K ),
  XX(SvSoundHaallVanster,           KWavKeepLeft ),
  XX(SvSoundHaallHoger,             KWavKeepRight ),
  XX(SvSoundIRondellen,             KWavRoundAbout ),
  XX(SvSoundDuHarAvvikitFranRutten, KWavOffTrack ),
  XX(SvSoundDuKorAtFelHall,         KWavWrongDirection ),
  XX(SvSoundDuArFrammeVidDinDest,   KWavFinish ),
  XX(SvSoundVidDestinationen,       KWavFinalAproach ),
  XX(SvSoundKorRaktFram,            KWavStraight ),
  XX(SvSoundKorAv,                  KWavExit ),
  XX(SvSoundKorUr,                  KWavExitInRdbt ),
  XX(SvSoundUtfarten,               KWavExitOfRdbt ),
  XX(SvSoundTa,                     KWavTake ),
  XX(SvSoundGorEnUSvang,            KWavUTurn ),
  XX(SvSoundForsta,                 KWav1st ),
  XX(SvSoundAndra,                  KWav2nd ),
  XX(SvSoundTredje,                 KWav3rd ),
  XX(SvSoundFjarde,                 KWav4th ),
  XX(SvSoundFemte,                  KWav5th ),
  XX(SvSoundSjatte,                 KWav6th ),
  XX(SvSoundSjunde,                 KWav7th ),
  XX(SvSoundAttonde,                KWav8th ),
  XX(SvSoundOchDarefter,            KWavAndThen ),
  XX(SvSoundDirekt,                 KWavImmediately ),
  XX(SvSoundCamera,                 KWavSeSoundKamera ),

#ifdef USE_OLD_ENGLISH
   /* English */
  // The clips must be sorted in AudioClipEnum-order
  XX(EnSoundTheFirst,               KWav1st ),
  XX(EnSoundTheSecond,              KWav2nd ),
  XX(EnSoundTheThird,               KWav3rd ),
  XX(EnSoundTheFourth,              KWav4th ),
  XX(EnSoundTheFifth,               KWav5th ),
  XX(EnSoundTheSixth,               KWav6th ),
  XX(EnSoundTheSeventh,             KWav7th ),
  XX(EnSoundTheEighth,              KWav8th ),
  XX(EnSoundAndThen,                KWavAndThen ),
  XX(EnSoundAtTheDestination,       KWavFinalAproach ),
  XX(EnSoundDriveStraightAhead,     KWavStraight ),
  XX(EnSoundExit,                   KWavExit ),
  XX(EnSoundYouHaveReachedYourDestination, KWavFinish ),
  XX(EnSoundHere,                   KWavHere ),
  XX(EnSoundInOneHundredMeters,     KWav100 ),
  XX(EnSoundInOneKilometer,         KWav1K ),
  XX(EnSoundInTwohundredMeters,     KWav200 ),
  XX(EnSoundInTwentyfiveMeters,     KWav25 ),
  XX(EnSoundInTwoKilometers,        KWav2K ),
  XX(EnSoundInFiftyMeters,          KWav50 ),
  XX(EnSoundInFivehundredMeters,    KWav500 ),
  XX(EnSoundInTheRoundabout,        KWavRoundAbout ),
  XX(EnSoundKeepLeft,               KWavKeepLeft ),
  XX(EnSoundKeepRight,              KWavKeepRight ),
  XX(EnSoundTake,                   KWavTake ),
  XX(EnSoundTurnLeft,               KWavTurnLeft ),
  XX(EnSoundTurnRight,              KWavTurnRight ),
  XX(EnSoundUturn,                  KWavUTurn ),
  XX(EnSoundYouAreDrivingInTheWrongDirection, KWavWrongDirection ),
  XX(EnSoundYouAreNowOffTrack,      KWavOffTrack ),
  XX(EnSoundCamera,                 KWavEnSoundCamera),
#else
  // The clips must be sorted in AudioClipEnum-order
  XX(EnSoundIn100Feet,              KWavEnSoundIn100Feet ),
  XX(EnSoundIn200Feet,              KWavEnSoundIn200Feet ),
  XX(EnSoundIn500Feet,              KWavEnSoundIn500Feet ),
  XX(EnSoundInAQuarterOfAMile,      KWavEnSoundInAQuarterOfAMile ),
  XX(EnSoundInHalfAMile,            KWavEnSoundInHalfAMile ),
  XX(EnSoundIn1Mile,                KWavEnSoundIn1Mile ),
  XX(EnSoundIn2Miles,               KWavEnSoundIn2Miles ),
  XX(EnSoundIn2Kilometers,          KWavEnSoundIn2Kilometers ),
  XX(EnSoundIn1Kilometer,           KWavEnSoundIn1Kilometer ),
  XX(EnSoundIn500Meters,            KWavEnSoundIn500Meters ),
  XX(EnSoundIn200Meters,            KWavEnSoundIn200Meters ),
  XX(EnSoundIn100Meters,            KWavEnSoundIn100Meters ),
  XX(EnSoundIn50Meters,             KWavEnSoundIn50Meters ),
  XX(EnSoundIn25Meters,             KWavEnSoundIn25Meters ),
  XX(EnSoundIn500Yards,             KWavEnSoundIn500Yards ),
  XX(EnSoundIn200Yards,             KWavEnSoundIn200Yards ),
  XX(EnSoundIn100Yards,             KWavEnSoundIn100Yards ),
  XX(EnSoundIn50Yards,              KWavEnSoundIn50Yards ),
  XX(EnSoundIn25Yards,              KWavEnSoundIn25Yards ),
  XX(EnSoundTheFirst,               KWavEnSoundTheFirst ),
  XX(EnSoundTheSecond,              KWavEnSoundTheSecond ),
  XX(EnSoundTheThird,               KWavEnSoundTheThird ),
  XX(EnSoundTheFourth,              KWavEnSoundTheFourth ),
  XX(EnSoundTheFifth,               KWavEnSoundTheFifth ),
  XX(EnSoundTheSixth,               KWavEnSoundTheSixth ),
  XX(EnSoundTheSeventh,             KWavEnSoundTheSeventh ),
  XX(EnSoundTheEighth,              KWavEnSoundTheEighth ),
  XX(EnSoundAndThen,                KWavEnSoundAndThen ),         
  XX(EnSoundAtTheDestination,       KWavEnSoundAtTheDestination ),
  XX(EnSoundAtTheRoundabout,        KWavEnSoundAtTheRoundabout ),
  XX(EnSoundDriveStraightAhead,     KWavEnSoundDriveStraightAhead ),
  XX(EnSoundExitNoun,               KWavEnSoundExitNoun ),
  XX(EnSoundExitVerb,               KWavEnSoundExitVerb ),
  XX(EnSoundHere,                   KWavEnSoundHere ),
  XX(EnSoundImmediately,            KWavEnSoundImmediately ),
  XX(EnSoundKeepLeft,               KWavEnSoundKeepLeft ),
  XX(EnSoundKeepRight,              KWavEnSoundKeepRight ),
  XX(EnSoundMakeAUTurn,             KWavEnSoundMakeAUTurn ),
  XX(EnSoundPleaseTurnAround,       KWavEnSoundPleaseTurnAround ),
  XX(EnSoundTake,                   KWavEnSoundTake ),
  XX(EnSoundTurnLeft,               KWavEnSoundTurnLeft ),
  XX(EnSoundTurnRight,              KWavEnSoundTurnRight ),
  XX(EnSoundYouAreNowOffTrack,      KWavEnSoundYouAreNowOffTrack ),
  XX(EnSoundYouHaveReachedYourDestination, KWavEnSoundYouHaveReachedYourDestination ),
  XX(EnSoundCamera, KWavEnSoundCamera),
#endif

  /* German */
  // The clips must be sorted in AudioClipEnum-order
  XX(DeSoundDieErste,                KWavDeSoundDieErste ),
  XX(DeSoundDieZweite,               KWavDeSoundDieZweite ),
  XX(DeSoundDieDritte,               KWavDeSoundDieDritte ),
  XX(DeSoundDieVierte,               KWavDeSoundDieVierte ),
  XX(DeSoundDieFunfte,               KWavDeSoundDieFunfte ),
  XX(DeSoundDieSechte,               KWavDeSoundDieSechte ),
  XX(DeSoundDieSibte,                KWavDeSoundDieSibte ),  
  XX(DeSoundDieAchrte,               KWavDeSoundDieAchrte ),
  XX(DeSoundUndDann,                 KWavDeSoundUndDann ),
  XX(DeSoundAmZiel,                  KWavDeSoundAmZiel ),
  XX(DeSoundGeradeausWeiterfahren,   KWavDeSoundGeradeausWeiterfahren ),
  XX(DeSoundAusfahrt,                KWavDeSoundAusfahrt ),
  XX(DeSoundAbfahren,                KWavDeSoundAbfahren ),
  XX(DeSoundSieHabenDasZielErreicht, KWavDeSoundSieHabenDasZielErreicht ),
  XX(DeSoundHier,                    KWavDeSoundHier ),
  XX(DeSoundInFunfUndZwanzigMetern,  KWavDeSoundInFunfUndZwanzigMetern ),
  XX(DeSoundInFunfzigMetern,         KWavDeSoundInFunfzigMetern ),
  XX(DeSoundInEinhundertMetern,      KWavDeSoundInEinhundertMetern ),
  XX(DeSoundInZweihundertMetern,     KWavDeSoundInZweihundertMetern ),
  XX(DeSoundInFunfhundertMetern,     KWavDeSoundInFunfhundertMetern ),
  XX(DeSoundInEinemKilometer,        KWavDeSoundInEinemKilometer ),
  XX(DeSoundInZweiKilometern,        KWavDeSoundInZweiKilometern ),
  XX(DeSoundImKreisel,               KWavDeSoundImKreisel ),
  XX(DeSoundLinksHalten,             KWavDeSoundLinksHalten ),
  XX(DeSoundRechtsHalten,            KWavDeSoundRechtsHalten ),
  XX(DeSoundLinksAbbiegen,           KWavDeSoundLinksAbbiegen ),
  XX(DeSoundRechtsAbbiegen,          KWavDeSoundRechtsAbbiegen ),
  XX(DeSoundSieFahrenJetztInDieFalsheRichtung, KWavDeSoundSieFahrenJetztInDieFalsheRichtung ),
  XX(DeSoundSieHabenJetztDieGeplanteRouteVerlassen, KWavDeSoundSieHabenJetztDieGeplanteRouteVerlassen ),  
  XX(DeSoundBitteWenden,             KWavDeSoundBitteWenden ),
  XX(DeSoundWennMoglichBitteWenden,  KWavDeSoundWennMoglichBitteWenden ),
  XX(DeSoundSofort,                  KWavDeSoundSofort ),
  XX(DeSoundCamera,                  KWavDeSoundCamera ),

  /* Danish */
  // The clips must be sorted in AudioClipEnum-order
  XX(DaSoundDenForste,                 KWav1st ),         
  XX(DaSoundDenAnnen,                  KWav2nd ),
  XX(DaSoundDenTredje,                 KWav3rd ),
  XX(DaSoundDenFjerre,                 KWav4th ),
  XX(DaSoundDenFemte,                  KWav5th ),
  XX(DaSoundDenSjaette,                KWav6th ),
  XX(DaSoundDenSuvene,                 KWav7th ),
  XX(DaSoundDenAttande,                KWav8th ),
  XX(DaSoundDarefter,                  KWavAndThen ),
  XX(DaSoundKorLigeUd,                 KWavStraight ),
  XX(DaSoundVej,                       KWavExitInRdbt ),
  XX(DaSoundFrakorsel,                 KWavExitOfRdbt ),
  XX(DaSoundDuErFrammeVidMalet,        KWavFinish ),
  XX(DaSoundNu,                        KWavHere ),
  XX(DaSoundOmEttHundredeMeter,        KWav100 ),
  XX(DaSoundOmEnKilometer,             KWav1K ),
  XX(DaSoundOmTuHundredeMeter,         KWav200 ),
  XX(DaSoundOmFemOgTuveMeter,          KWav25 ),
  XX(DaSoundOmTuKilometer,             KWav2K ),
  XX(DaSoundOmHalvtredsMeter,          KWav50 ),
  XX(DaSoundOmFemhundredeMeter,        KWav500 ),
  XX(DaSoundIRundkorseln,              KWavRoundAbout ),
  XX(DaSoundHallTillVenstre,           KWavKeepLeft ),
  XX(DaSoundHallTillHojre,             KWavKeepRight ),        
  XX(DaSoundTa,                        KWavTake ),
  XX(DaSoundMaletErRaettfram,          KWavFinalAproach ),
  XX(DaSoundDrejTillVenstre,           KWavTurnLeft ),
  XX(DaSoundDrejTillHojre,             KWavTurnRight ),
  XX(DaSoundLavEnUVandning,            KWavUTurn ),
  XX(DaSoundDuKorIDenForkerteRaktning, KWavWrongDirection ),
  XX(DaSoundDuErAvviketFraRutten,      KWavOffTrack ),
  XX(DaSoundMeddetsamme,               KWavImmediately ),
  XX(DaSoundCamera,                    KWavEOTurn ),
  
  /* Italian */
  // The clips must be sorted in AudioClipEnum-order
  XX(ItSoundLaPrima,                   KWavItSoundLaPrima ),
  XX(ItSoundLaSeconda,                 KWavItSoundLaSeconda ),
  XX(ItSoundLaTerza,                   KWavItSoundLaTerza ),
  XX(ItSoundLaQuarta,                  KWavItSoundLaQuarta ),
  XX(ItSoundLaQuinta,                  KWavItSoundLaQuinta ),
  XX(ItSoundLaSesta,                   KWavItSoundLaSesta ),
  XX(ItSoundLaSettima,                 KWavItSoundLaSettima ),
  XX(ItSoundLottava,                   KWavItSoundLottava ),
  XX(ItSoundQuindi,                    KWavItSoundQuindi ),
  XX(ItSoundADestinazione,             KWavItSoundADestinazione ),
  XX(ItSoundProcedoDiritto,            KWavItSoundProcedoDiritto ),
  XX(ItSoundUscita,                    KWavItSoundUscita ),
  XX(ItSoundEsci,                      KWavItSoundEsci ),
  XX(ItSoundHaiRaggiunto,              KWavItSoundHaiRaggiunto ),
  XX(ItSoundQui,                       KWavItSoundQui ),
  XX(ItSoundTra25Metri,                KWavItSoundTra25Metri ),
  XX(ItSoundTra50Metri,                KWavItSoundTra50Metri ),
  XX(ItSoundTra100Metri,               KWavItSoundTra100Metri ),
  XX(ItSoundTra200Metri,               KWavItSoundTra200Metri ),
  XX(ItSoundTra500Metri,               KWavItSoundTra500Metri ),
  XX(ItSoundTra1Chilometro,            KWavItSoundTra1Chilometro ),
  XX(ItSoundTra2Chilometri,            KWavItSoundTra2Chilometri ),
  XX(ItSoundAllaRotonda,               KWavItSoundAllaRotonda ),
  XX(ItSoundMantieniLaSinistra,        KWavItSoundMantieniLaSinistra ),
  XX(ItSoundMantieniLaDestra,          KWavItSoundMantieniLaDestra ),
  XX(ItSoundSvoltaASinistra,           KWavItSoundSvoltaASinistra ),
  XX(ItSoundSvoltaADestra,             KWavItSoundSvoltaADestra ),
  XX(ItSoundSeiFuoriStrada,            KWavItSoundSeiFuoriStrada ),
  XX(ItSoundStaiProcedendo,            KWavItSoundStaiProcedendo ),
  XX(ItSoundQuandoPossibile,           KWavItSoundQuandoPossibile ),
  XX(ItSoundImmediatamente,            KWavItSoundImmediatamente ),
  XX(ItSoundPrendi,                    KWavItSoundPrendi ),
  XX(ItSoundEsciDallAutostrada,        KWavItSoundEsciDallAutostrada ),
  XX(ItSoundEseguiUnInversioneAU,      KWavItSoundEseguiUnInversioneAU ),
  XX(ItSoundCamera,                    KWavItSoundCamera ),

  /* French */
  // The clips must be sorted in AudioClipEnum-order
  XX(FrSoundLaPremiere,                KWavFrSoundLaPremiere ),
  XX(FrSoundLaDeuxieme,                KWavFrSoundLaDeuxieme ),
  XX(FrSoundLaTroisieme,               KWavFrSoundLaTroisieme ),
  XX(FrSoundLaQuatrieme,               KWavFrSoundLaQuatrieme ),
  XX(FrSoundLaCinquieme,               KWavFrSoundLaCinquieme ),
  XX(FrSoundLaSixieme,                 KWavFrSoundLaSixieme ),
  XX(FrSoundLaSeptieme,                KWavFrSoundLaSeptieme ),
  XX(FrSoundLaHuitieme,                KWavFrSoundLaHuitieme ),
  XX(FrSoundImmediatement,             KWavFrSoundImmediatement ),
  XX(FrSoundIci,                       KWavFrSoundIci ),
  XX(FrSoundA25Metres,                 KWavFrSoundA25Metres ),
  XX(FrSoundA50Metres,                 KWavFrSoundA50Metres ),
  XX(FrSoundA100Metres,                KWavFrSoundA100Metres ),
  XX(FrSoundA200Metres,                KWavFrSoundA200Metres ),
  XX(FrSoundA500Metres,                KWavFrSoundA500Metres ),
  XX(FrSoundA1Kilometre,               KWavFrSoundA1Kilometre ),
  XX(FrSoundA2Kilometres,              KWavFrSoundA2Kilometres ),
  XX(FrSoundADestination,              KWavFrSoundADestination ),
  XX(FrSoundADroite,                   KWavFrSoundADroite ),
  XX(FrSoundAGauche,                   KWavFrSoundAGauche ),
  XX(FrSoundAPresentVousEtes,          KWavFrSoundAPresentVousEtes ),  
  XX(FrSoundAuRondpoint,               KWavFrSoundAuRondpoint ),
  XX(FrSoundConduisezToutDroit,        KWavFrSoundConduisezToutDroit ),
  XX(FrSoundContinuez,                 KWavFrSoundContinuez ),
  XX(FrSoundDesQuePossible,            KWavFrSoundDesQuePossible ),
  XX(FrSoundFaitesDemitour,            KWavFrSoundFaitesDemitour ),
  XX(FrSoundPrenez,                    KWavFrSoundPrenez ),
  XX(FrSoundQuittezLautoroute,         KWavFrSoundQuittezLautoroute ),
  XX(FrSoundSortez,                    KWavFrSoundSortez ),
  XX(FrSoundSortie,                    KWavFrSoundSortie ),
  XX(FrSoundTournez,                   KWavFrSoundTournez ),
  XX(FrSoundVousEtesArrive,            KWavFrSoundVousEtesArrive ),
  XX(FrSoundPuis,                      KWavFrSoundPuis ),
  XX(FrSoundCamera,                    KWavFrSoundCamera ),
  
  /* Spanish */
  // The clips must be sorted in AudioClipEnum-order
  XX(EsSoundLaPrimera,                 KWavEsSoundLaPrimera ),
  XX(EsSoundLaSegunda,                 KWavEsSoundLaSegunda ),
  XX(EsSoundLaTercera,                 KWavEsSoundLaTercera ),
  XX(EsSoundLaCuarta,                  KWavEsSoundLaCuarta ),
  XX(EsSoundLaQuinta,                  KWavEsSoundLaQuinta ),
  XX(EsSoundLaSexta,                   KWavEsSoundLaSexta ),
  XX(EsSoundLaSeptima,                 KWavEsSoundLaSeptima ),
  XX(EsSoundLaOctava,                  KWavEsSoundLaOctava ),
  XX(EsSoundAqui,                      KWavEsSoundAqui ),
  XX(EsSoundCambiaDeSentido,           KWavEsSoundCambiaDeSentido ),
  XX(EsSoundConduceTodoRecto,          KWavEsSoundConduceTodoRecto ),
  XX(EsSoundCuandoPuedas,              KWavEsSoundCuandoPuedas ),
  XX(EsSoundElDestino,                 KWavEsSoundElDestino ),
  XX(EsSoundEn100Metros,               KWavEsSoundEn100Metros ),
  XX(EsSoundEn1Kilometro,              KWavEsSoundEn1Kilometro ),
  XX(EsSoundEn200Metros,               KWavEsSoundEn200Metros ),
  XX(EsSoundEn25Metros,                KWavEsSoundEn25Metros ),
  XX(EsSoundEn2Kilometros,             KWavEsSoundEn2Kilometros ),
  XX(EsSoundEn500Metros,               KWavEsSoundEn500Metros ),
  XX(EsSoundEn50Metros,                KWavEsSoundEn50Metros ),
  XX(EsSoundEnElDestino,               KWavEsSoundEnElDestino ),
  XX(EsSoundEnLaRotonda,               KWavEsSoundEnLaRotonda ),
  XX(EsSoundEstasConduciendo,          KWavEsSoundEstasConduciendo ),
  XX(EsSoundEstasFueraDeLaRuta,        KWavEsSoundEstasFueraDeLaRuta ),
  XX(EsSoundGiraALaDerecha,            KWavEsSoundGiraALaDerecha ),
  XX(EsSoundGiraALaIzquierda,          KWavEsSoundGiraALaIzquierda ),
  XX(EsSoundHasLlegadoATuDestino,      KWavEsSoundHasLlegadoATuDestino ),
  XX(EsSoundInmeditamente,             KWavEsSoundInmeditamente ),
  XX(EsSoundMantenteALaDerecha,        KWavEsSoundMantenteALaDerecha ),
  XX(EsSoundMantenteALaIzquierda,      KWavEsSoundMantenteALaIzquierda ),
  XX(EsSoundSalida,                    KWavEsSoundSalida ),
  XX(EsSoundSal,                       KWavEsSoundSal ),
  XX(EsSoundSalDeLaAutopista,          KWavEsSoundSalDeLaAutopista ),
  XX(EsSoundToma,                      KWavEsSoundToma ),
  XX(EsSoundYDespues,                  KWavEsSoundYDespues ),
  XX(EsSoundCamera,                    KWavEsSoundCamera ),
  
  /* Dutch */
  // The clips must be sorted in AudioClipEnum-order
  XX(NlSoundAfrit,                       KWavNlSoundAfrit ),
  XX(NlSoundBijDeBestemming,             KWavNlSoundBijDeBestemming ),
  XX(NlSoundDeEerste,                    KWavNlSoundDeEerste ),
  XX(NlSoundDeTweede,                    KWavNlSoundDeTweede ),
  XX(NlSoundDeDerde,                     KWavNlSoundDeDerde ),
  XX(NlSoundDeVierde,                    KWavNlSoundDeVierde ),
  XX(NlSoundDeVijfde,                    KWavNlSoundDeVijfde ),
  XX(NlSoundDeZesde,                     KWavNlSoundDeZesde ),
  XX(NlSoundDeZevende,                   KWavNlSoundDeZevende ),
  XX(NlSoundDeAchtste,                   KWavNlSoundDeAchtste ),
  XX(NlSoundEnDan,                       KWavNlSoundEnDan ),
  XX(NlSoundHier,                        KWavNlSoundHier ),
  XX(NlSoundKeerOmIndienMogelijk,        KWavNlSoundKeerOmIndienMogelijk ),
  XX(NlSoundLinksAfslaan,                KWavNlSoundLinksAfslaan ),
  XX(NlSoundLinksHouden,                 KWavNlSoundLinksHouden ),
  XX(NlSoundLinks,                       KWavNlSoundLinks ),
  XX(NlSoundNa100Meter,                  KWavNlSoundNa100Meter ),
  XX(NlSoundNa1Kilometer,                KWavNlSoundNa1Kilometer ),
  XX(NlSoundNa200Meter,                  KWavNlSoundNa200Meter ),
  XX(NlSoundNa25Meter,                   KWavNlSoundNa25Meter ),
  XX(NlSoundNa2Kilometer,                KWavNlSoundNa2Kilometer ),
  XX(NlSoundNa500Meter,                  KWavNlSoundNa500Meter ),
  XX(NlSoundNa50Meter,                   KWavNlSoundNa50Meter ),
  XX(NlSoundNaarLinks,                   KWavNlSoundNaarLinks ),
  XX(NlSoundNaarRechts,                  KWavNlSoundNaarRechts ),
  XX(NlSoundOmkeren,                     KWavNlSoundOmkeren ),
  XX(NlSoundOnmiddellijk,                KWavNlSoundOnmiddellijk ),
  XX(NlSoundOpDeRotonde,                 KWavNlSoundOpDeRotonde ),
  XX(NlSoundRechtdoorRijden,             KWavNlSoundRechtdoorRijden ),
  XX(NlSoundRechtsAfslaan,               KWavNlSoundRechtsAfslaan ),
  XX(NlSoundRechtsHouden,                KWavNlSoundRechtsHouden ),
  XX(NlSoundRechts,                      KWavNlSoundRechts ),             
  XX(NlSoundSnelwegVerlaten,             KWavNlSoundSnelwegVerlaten ),
  XX(NlSoundUBevindtZichNuBuitenDeRoute, KWavNlSoundUBevindtZichNuBuitenDeRoute ),
  XX(NlSoundUHebtUwBestemmingBereikt,    KWavNlSoundUHebtUwBestemmingBereikt ),
  XX(NlSoundURijdtInDeVerkeerdeRichting, KWavNlSoundURijdtInDeVerkeerdeRichting ),
  XX(NlSoundCamera,                      KWavNlSoundCamera ),
  /* Finnish */
  // The clips must be sorted in AudioClipEnum-order
  XX(FiSound25Metrin,                    KWavFiSound25Metrin ),
  XX(FiSound50Metrin,                    KWavFiSound50Metrin ),
  XX(FiSound100Metrin,                   KWavFiSound100Metrin ),
  XX(FiSound200Metrin,                   KWavFiSound200Metrin ),
  XX(FiSound500Metrin,                   KWavFiSound500Metrin ),
  XX(FiSound1Kilometrin,                 KWavFiSound1Kilometrin ),
  XX(FiSound2Kilometrin,                 KWavFiSound2Kilometrin ),
  XX(FiSoundEnsimmainen,                 KWavFiSoundEnsimmainen ),
  XX(FiSoundToinen,                      KWavFiSoundToinen ),
  XX(FiSoundKolmas,                      KWavFiSoundKolmas ),
  XX(FiSoundNeljas,                      KWavFiSoundNeljas ),
  XX(FiSoundViides,                      KWavFiSoundViides ),
  XX(FiSoundKuudes,                      KWavFiSoundKuudes ),
  XX(FiSoundSeitsemas,                   KWavFiSoundSeitsemas ),
  XX(FiSoundKahdeksas,                   KWavFiSoundKahdeksas ),
  XX(FiSoundEtSeuraaAnnettuaReittia,     KWavFiSoundEtSeuraaAnnettuaReittia ),
  XX(FiSoundJaSitten,                    KWavFiSoundJaSitten ),
  XX(FiSoundKaannyOikealle,              KWavFiSoundKaannyOikealle ),
  XX(FiSoundKaannyVasemmalle,            KWavFiSoundKaannyVasemmalle ),
  XX(FiSoundJalkeen,                     KWavFiSoundJalkeen ),
  XX(FiSoundPaassa,                      KWavFiSoundPaassa ),
  XX(FiSoundLiikenneympyrasta,           KWavFiSoundLiikenneympyrasta ),
  XX(FiSoundOletTullutPerille,           KWavFiSoundOletTullutPerille ),
  XX(FiSoundPoistu,                      KWavFiSoundPoistu ),
  XX(FiSoundPysyOikealla,                KWavFiSoundPysyOikealla ),
  XX(FiSoundPysyVasemmalla,              KWavFiSoundPysyVasemmalla ),
  XX(FiSoundTassa,                       KWavFiSoundTassa ),
  XX(FiSoundTasta,                       KWavFiSoundTasta ),
  XX(FiSoundTeeUKaannosHeti,             KWavFiSoundTeeUKaannosHeti ),
  XX(FiSoundUloskaynti,                  KWavFiSoundUloskaynti ),
  XX(FiSoundValitseLiikenneympyrasta,    KWavFiSoundValitseLiikenneympyrasta ),
  XX(FiSoundValittomasti,                KWavFiSoundValittomasti ),
  XX(FiSoundKaanny,                      KWavFiSoundKaanny ),
  XX(FiSoundYmpari,                      KWavFiSoundYmpari ),
  XX(FiSoundMaaranpaasiOn,               KWavFiSoundMaaranpaasiOn ),
  XX(FiSoundSuoraanEteenpain,            KWavFiSoundSuoraanEteenpain ),
  XX(FiSoundVasemmalle,                  KWavFiSoundVasemmalle ),
  XX(FiSoundOikealle,                    KWavFiSoundOikealle ),
  XX(FiSoundAjaLiikenneymparastaSouraanEteenpain, KWavFiSoundAjaLiikenneymparastaSouraanEteenpain ),
   XX(FiSoundCamera, KWavFiSoundCamera ),
  
  /* Norwegian */
  // The clips must be sorted in AudioClipEnum-order
  XX(NoSound25,                          KWavNoSound25 ),
  XX(NoSound50,                          KWavNoSound50 ),
  XX(NoSound100,                         KWavNoSound100 ),
  XX(NoSound200,                         KWavNoSound200 ),
  XX(NoSound500,                         KWavNoSound500 ),
  XX(NoSound1,                           KWavNoSound1 ),
  XX(NoSound2,                           KWavNoSound2 ),
  XX(NoSoundForste,                      KWavNoSoundForste ),
  XX(NoSoundAndre,                       KWavNoSoundAndre ),
  XX(NoSoundTredje,                      KWavNoSoundTredje ),
  XX(NoSoundFjerde,                      KWavNoSoundFjerde ),
  XX(NoSoundFemte,                       KWavNoSoundFemte ),
  XX(NoSoundSjette,                      KWavNoSoundSjette ),
  XX(NoSoundSjuende,                     KWavNoSoundSjuende ),
  XX(NoSoundAttende,                     KWavNoSoundAttende ),
  XX(NoSoundAvkjoring,                   KWavNoSoundAvkjoring ),
  XX(NoSoundDestinasjonenErRettFrem,     KWavNoSoundDestinasjonenErRettFrem ),
  XX(NoSoundDuErFremmeVedDestinasjonen,  KWavNoSoundDuErFremmeVedDestinasjonen ),
  XX(NoSoundDuErUtenforRuten,            KWavNoSoundDuErUtenforRuten ),
  XX(NoSoundForlatHovedveien,            KWavNoSoundForlatHovedveien ),
  XX(NoSoundGjorEnUSvingNarDetBlirMulig, KWavNoSoundGjorEnUSvingNarDetBlirMulig ),
  XX(NoSoundHer,                         KWavNoSoundHer ),
  XX(NoSoundHold,                        KWavNoSoundHold ),
  XX(NoSoundIEndenAvGaten,               KWavNoSoundIEndenAvGaten ),
  XX(NoSoundIRundkjoringen,              KWavNoSoundIRundkjoringen ),
  XX(NoSoundKilometer,                   KWavNoSoundKilometer ),
  XX(NoSoundKjorAv,                      KWavNoSoundKjorAv ),
  XX(NoSoundKjorHeleVeienRundt,          KWavNoSoundKjorHeleVeienRundt ),
  XX(NoSoundKjorRettFrem,                KWavNoSoundKjorRettFrem ),
  XX(NoSoundKjorUt,                      KWavNoSoundKjorUt ),
  XX(NoSoundMeter,                       KWavNoSoundMeter ),
  XX(NoSoundMiles,                       KWavNoSoundMiles ),
  XX(NoSoundOgSa,                        KWavNoSoundOgSa ),
  XX(NoSoundOm,                          KWavNoSoundOm ),
  XX(NoSoundSving,                       KWavNoSoundSving ),
  XX(NoSoundTa,                          KWavNoSoundTa ),
  XX(NoSoundTilHoyre,                    KWavNoSoundTilHoyre ),
  XX(NoSoundTilVenstre,                  KWavNoSoundTilVenstre ),
  XX(NoSoundUmiddelbart,                 KWavNoSoundUmiddelbart ),
  XX(NoSoundVedDestinasjonen,            KWavNoSoundVedDestinasjonen ),
  XX(NoSoundVennligstSnu,                KWavNoSoundVennligstSnu ),
  XX(NoSoundVenstre,                     KWavNoSoundVenstre ),
  XX(NoSoundYards,                       KWavNoSoundYards ),
  XX(NoSoundCamera,                      KWavNoSoundCamera ),
};

const AudioClipTableTableDriven::int_to_clipname_t
OldAudioClipTable::m_staticClipTable2 [] =
{ /* American English */
  // The clips must be sorted in AudioClipEnum-order
  XX(UsSoundIn100Feet,                   KWavUsSoundIn100Feet ),
  XX(UsSoundIn200Feet,                   KWavUsSoundIn200Feet ),
  XX(UsSoundIn500Feet,                   KWavUsSoundIn500Feet ),
  XX(UsSoundInAQuarterOfAMile,           KWavUsSoundInAQuarterOfAMile ),
  XX(UsSoundInHalfAMile,                 KWavUsSoundInHalfAMile ),
  XX(UsSoundIn1Mile,                     KWavUsSoundIn1Mile ),
  XX(UsSoundIn2Miles,                    KWavUsSoundIn2Miles ),
  XX(UsSoundIn2Kilometers,               KWavUsSoundIn2Kilometers ),
  XX(UsSoundIn1Kilometer,                KWavUsSoundIn1Kilometer ),
  XX(UsSoundIn500Meters,                 KWavUsSoundIn500Meters ),
  XX(UsSoundIn200Meters,                 KWavUsSoundIn200Meters ),
  XX(UsSoundIn100Meters,                 KWavUsSoundIn100Meters ),
  XX(UsSoundIn50Meters,                  KWavUsSoundIn50Meters ),
  XX(UsSoundIn25Meters,                  KWavUsSoundIn25Meters ),
  XX(UsSoundIn500Yards,                  KWavUsSoundIn500Yards ),
  XX(UsSoundIn200Yards,                  KWavUsSoundIn200Yards ),
  XX(UsSoundIn100Yards,                  KWavUsSoundIn100Yards ),
  XX(UsSoundIn50Yards,                   KWavUsSoundIn50Yards ),
  XX(UsSoundIn25Yards,                   KWavUsSoundIn25Yards ),
  XX(UsSoundTheFirst,                    KWavUsSoundTheFirst ),
  XX(UsSoundTheSecond,                   KWavUsSoundTheSecond ),
  XX(UsSoundTheThird,                    KWavUsSoundTheThird ),
  XX(UsSoundTheFourth,                   KWavUsSoundTheFourth ),
  XX(UsSoundTheFifth,                    KWavUsSoundTheFifth ),
  XX(UsSoundTheSixth,                    KWavUsSoundTheSixth ),
  XX(UsSoundTheSeventh,                  KWavUsSoundTheSeventh ),
  XX(UsSoundTheEighth,                   KWavUsSoundTheEighth ),
  XX(UsSoundAndThen,                     KWavUsSoundAndThen ),
  XX(UsSoundAtTheDestination,            KWavUsSoundAtTheDestination ),
  XX(UsSoundAtTheRoundabout,             KWavUsSoundAtTheRoundabout ),
  XX(UsSoundDriveStraightAhead,          KWavUsSoundDriveStraightAhead ),
  XX(UsSoundExitNoun,                    KWavUsSoundExitNoun ),
  XX(UsSoundExitVerb,                    KWavUsSoundExitVerb ),
  XX(UsSoundHere,                        KWavUsSoundHere ),
  XX(UsSoundImmediately,                 KWavUsSoundImmediately ),
  XX(UsSoundKeepLeft,                    KWavUsSoundKeepLeft ),
  XX(UsSoundKeepRight,                   KWavUsSoundKeepRight ),
  XX(UsSoundMakeAUTurn,                  KWavUsSoundMakeAUTurn ),
  XX(UsSoundPleaseTurnAround,            KWavUsSoundPleaseTurnAround ),
  XX(UsSoundTake,                        KWavUsSoundTake ),
  XX(UsSoundTurnLeft,                    KWavUsSoundTurnLeft ),
  XX(UsSoundTurnRight,                   KWavUsSoundTurnRight ),
  XX(UsSoundYouAreNowOffTrack,           KWavUsSoundYouAreNowOffTrack ),
  XX(UsSoundYouHaveReachedYourDestination,    KWavUsSoundYouHaveReachedYourDestination ),
  XX(UsSoundCamera,    KWavUsSoundCamera ),
  
  /* Portuguese */
  // The clips must be sorted in AudioClipEnum-order
   XX(PtSound25Metros,                  KWavPtSound25Metros ),
   XX(PtSound50Metros,                  KWavPtSound50Metros ),
   XX(PtSound100Metros,                 KWavPtSound100Metros ),
   XX(PtSound200Metros,                 KWavPtSound200Metros ),
   XX(PtSound500Metros,                 KWavPtSound500Metros ),
   XX(PtSound1Quilometro,               KWavPtSound1Quilometro ),
   XX(PtSound2Quilometros,              KWavPtSound2Quilometros ),
   XX(PtSoundNaPrimeira,                KWavPtSoundNaPrimeira ),
   XX(PtSoundNaSegunda,                 KWavPtSoundNaSegunda ),
   XX(PtSoundNaTerceira,                KWavPtSoundNaTerceira ),
   XX(PtSoundNaQuarta,                  KWavPtSoundNaQuarta ),
   XX(PtSoundNaQuinta,                  KWavPtSoundNaQuinta ),
   XX(PtSoundNaSexta,                   KWavPtSoundNaSexta ),
   XX(PtSoundNaSetima,                  KWavPtSoundNaSetima ),
   XX(PtSoundNaOitava,                  KWavPtSoundNaOitava ),
   XX(PtSoundA,                         KWavPtSoundA ),
   XX(PtSoundAqui,                      KWavPtSoundAqui ),
   XX(PtSoundDaquiA,                    KWavPtSoundDaquiA ),
   XX(PtSoundEASeguir,                  KWavPtSoundEASeguir ),
   XX(PtSoundFacaInversaoDeMarcha,      KWavPtSoundFacaInversaoDeMarcha ),
   XX(PtSoundImediatamente,             KWavPtSoundImediatamente ),
   XX(PtSoundMantenhaSeADireita,        KWavPtSoundMantenhaSeADireita ),
   XX(PtSoundMantenhaSeAEsquerda,       KWavPtSoundMantenhaSeAEsquerda ),
   XX(PtSoundNaRotunda,                 KWavPtSoundNaRotunda ),
   XX(PtSoundNoDestino,                 KWavPtSoundNoDestino ),
   XX(PtSoundQuandoPossivel,            KWavPtSoundQuandoPossivel ),
   XX(PtSoundSaia,                      KWavPtSoundSaia ),
   XX(PtSoundSaida,                     KWavPtSoundSaida ),
   XX(PtSoundSigaEmFrente,              KWavPtSoundSigaEmFrente ),
   XX(PtSoundVire,                      KWavPtSoundVire ),
   XX(PtSoundVireADireita,              KWavPtSoundVireADireita ),
   XX(PtSoundVireAEsquerda,             KWavPtSoundVireAEsquerda ),
   XX(PtSoundVoceChegouAoSeuDestino,    KWavPtSoundVoceChegouAoSeuDestino ),
   XX(PtSoundVoceEstaForaDeRota,        KWavPtSoundVoceEstaForaDeRota ),
    XX(PtSoundCamera,                    KWavPtSoundCamera ),
   
   /* Hungarian */
   // The clips must be sorted in AudioClipEnum-order
   XX(HuSound100,                       KWavHuSound100 ),
   XX(HuSound1,                         KWavHuSound1 ),
   XX(HuSound200,                       KWavHuSound200 ),
   XX(HuSound25,                        KWavHuSound25 ),
   XX(HuSound2,                         KWavHuSound2 ),
   XX(HuSound500,                       KWavHuSound500 ),
   XX(HuSound50,                        KWavHuSound50 ),
   XX(HuSoundAzElso,                    KWavHuSoundAzElso ),
   XX(HuSoundAMasodik,                  KWavHuSoundAMasodik ),
   XX(HuSoundAHarmadik,                 KWavHuSoundAHarmadik ),
   XX(HuSoundANegyedik,                 KWavHuSoundANegyedik ),
   XX(HuSoundAzOtodik,                  KWavHuSoundAzOtodik ),
   XX(HuSoundAHatodik,                  KWavHuSoundAHatodik ),
   XX(HuSoundAHetedik,                  KWavHuSoundAHetedik ),
   XX(HuSoundANyolcadik,                KWavHuSoundANyolcadik ),
   XX(HuSoundAKijaratig,                KWavHuSoundAKijaratig ),
   XX(HuSoundAKorforgalomnal,           KWavHuSoundAKorforgalomnal ),
   XX(HuSoundAKorforgalomban,           KWavHuSoundAKorforgalomban ),
   XX(HuSoundACelpontElottedVan,        KWavHuSoundACelpontElottedVan ),
   XX(HuSoundACelpontig,                KWavHuSoundACelpontig ),
   XX(HuSoundBalra,                     KWavHuSoundBalra ),
   XX(HuSoundEgyenesen,                 KWavHuSoundEgyenesen ),
   XX(HuSoundElertedACelpontot,         KWavHuSoundElertedACelpontot ),
   XX(HuSoundFel,                       KWavHuSoundFel ),
   XX(HuSoundForduljVisszaAmintLehet,   KWavHuSoundForduljVisszaAmintLehet ),
   XX(HuSoundForduljVissza,             KWavHuSoundForduljVissza ),
   XX(HuSoundFordulj,                   KWavHuSoundFordulj ),
   XX(HuSoundHajtsKi,                   KWavHuSoundHajtsKi ),
   XX(HuSoundItt,                       KWavHuSoundItt ),
   XX(HuSoundJardot,                    KWavHuSoundJardot ),
   XX(HuSoundJobbra,                    KWavHuSoundJobbra ),
   XX(HuSoundKijaraton,                 KWavHuSoundKijaraton ),
   XX(HuSoundKilometert,                KWavHuSoundKilometert ),
   XX(HuSoundLabat,                     KWavHuSoundLabat ),
   XX(HuSoundMajdUtana,                 KWavHuSoundMajdUtana ),
   XX(HuSoundMenjTovabbEgyenesen1,      KWavHuSoundMenjTovabbEgyenesen1 ),
   XX(HuSoundMenjTovabbEgyenesen2,      KWavHuSoundMenjTovabbEgyenesen2 ),
   XX(HuSoundMenj,                      KWavHuSoundMenj ),
   XX(HuSoundMerfoldet,                 KWavHuSoundMerfoldet ),
   XX(HuSoundMetert,                    KWavHuSoundMetert ),
   XX(HuSoundMostLetertelAzUtvonalrol,  KWavHuSoundMostLetertelAzUtvonalrol ),
   XX(HuSoundMost,                      KWavHuSoundMost ),
   XX(HuSoundNegyed,                    KWavHuSoundNegyed ),
   XX(HuSoundNyolcad,                   KWavHuSoundNyolcad ),
   XX(HuSoundRosszIranybaMesz,          KWavHuSoundRosszIranybaMesz ),
   XX(HuSoundTarts,                     KWavHuSoundTarts ),
   XX(HuSoundCamera,                    KWavHuSoundCamera ),


   /* Polish */
   // The clips must be sorted in AudioClipEnum-order
   XX(PlSoundWPierwszy,                 KWavPlSoundWPierwszy ),
   XX(PlSoundWDrugi,                    KWavPlSoundWDrugi ),
   XX(PlSoundWTrzeci,                   KWavPlSoundWTrzeci ),
   XX(PlSoundWCzwarty,                  KWavPlSoundWCzwarty ),
   XX(PlSoundWPiaty,                    KWavPlSoundWPiaty ),
   XX(PlSoundWSzusty,                   KWavPlSoundWSzusty ),
   XX(PlSoundWSiodmy,                   KWavPlSoundWSiodmy ),
   XX(PlSoundWOsmy,                     KWavPlSoundWOsmy ),
   XX(PlSoundTutaj,                     KWavPlSoundTutaj ),
   XX(PlSoundNatychmiast,               KWavPlSoundNatychmiast ),
   XX(PlSoundZa25Metrow,                KWavPlSoundZa25Metrow ),
   XX(PlSoundZa50Metrow,                KWavPlSoundZa50Metrow ),
   XX(PlSoundZa100Metrow,               KWavPlSoundZa100Metrow ),
   XX(PlSoundZa200Metrow,               KWavPlSoundZa200Metrow ),
   XX(PlSoundZa500Metrow,               KWavPlSoundZa500Metrow ),
   XX(PlSoundZa25Jardow,                KWavPlSoundZa25Jardow ),
   XX(PlSoundZa50Jardow,                KWavPlSoundZa50Jardow ),
   XX(PlSoundZa100Jardow,               KWavPlSoundZa100Jardow ),
   XX(PlSoundZa200Jardow,               KWavPlSoundZa200Jardow ),
   XX(PlSoundZa500Jardow,               KWavPlSoundZa500Jardow ),
   XX(PlSoundZa25Stop,                  KWavPlSoundZa25Stop ),
   XX(PlSoundZa50Stop,                  KWavPlSoundZa50Stop ),
   XX(PlSoundZa100Stop,                 KWavPlSoundZa100Stop ),
   XX(PlSoundZa200Stop,                 KWavPlSoundZa200Stop ),
   XX(PlSoundZa500Stop,                 KWavPlSoundZa500Stop ),
   XX(PlSoundZa1Kilometr,               KWavPlSoundZa1Kilometr ),
   XX(PlSoundZa2Kilometry,              KWavPlSoundZa2Kilometry ),
   XX(PlSoundZaPuMili,                  KWavPlSoundZaPuMili ),
   XX(PlSoundZaJednotwartoMili,         KWavPlSoundZaJednotwartoMili ),
   XX(PlSoundZaJednoossmoMili,          KWavPlSoundZaJednoossmoMili ),
   XX(PlSoundZa1Mile,                   KWavPlSoundZa1Mile ),
   XX(PlSoundZa2Mile,                   KWavPlSoundZa2Mile ),
   XX(PlSoundAPotem,                    KWavPlSoundAPotem ),
   XX(PlSoundDojechales,                KWavPlSoundDojechales ),
   XX(PlSoundJedz,                      KWavPlSoundJedz ),
   XX(PlSoundJedzProsto,                KWavPlSoundJedzProsto ),
   XX(PlSoundKamera,                    KWavPlSoundKamera ),
   XX(PlSoundNaRondzie,                 KWavPlSoundNaRondzie ),
   XX(PlSoundOstrzezenie,               KWavPlSoundOstrzezenie ),
   XX(PlSoundProszeZawrocic,            KWavPlSoundProszeZawrocic ),
   XX(PlSoundPunkt,                     KWavPlSoundPunkt ),
   XX(PlSoundSkrecWLewo,                KWavPlSoundSkrecWLewo ),
   XX(PlSoundSkrecWPrawo,               KWavPlSoundSkrecWPrawo ),
   XX(PlSoundTeraz,                     KWavPlSoundTeraz ),
   XX(PlSoundTrzymajSieLewej,           KWavPlSoundTrzymajSieLewej ),
   XX(PlSoundTrzymajSiePrawej,          KWavPlSoundTrzymajSiePrawej ),
   XX(PlSoundWlasnie,                   KWavPlSoundWlasnie ),
   XX(PlSoundWMiejscuPrzeznaczenia,     KWavPlSoundWMiejscuPrzeznaczenia ),
   XX(PlSoundZawrocGdyBedzieToMozliwe,  KWavPlSoundZawrocGdyBedzieToMozliwe ),
   XX(PlSoundZjazd,                     KWavPlSoundZjazd ),
   XX(PlSoundZjedz,                     KWavPlSoundZjedz ),
   XX(PlSoundZjedzZAutostrade,          KWavPlSoundZjedzZAutostrade ),
   XX(PlSoundCamera,                    KWavPlSoundCamera ),

   /* Czech */
   // The clips must be sorted in AudioClipEnum-order
   XX(CsSoundPrvnim,                    KWavCsSoundPrvnim),
   XX(CsSoundDruhym,                    KWavCsSoundDruhym),
   XX(CsSoundTretim,                    KWavCsSoundTretim),
   XX(CsSoundCtvrtym,                   KWavCsSoundCtvrtym),
   XX(CsSoundPatym,                     KWavCsSoundPatym),
   XX(CsSoundSestym,                    KWavCsSoundSestym),
   XX(CsSoundSedmym,                    KWavCsSoundSedmym),
   XX(CsSoundOsmym,                     KWavCsSoundOsmym),
   XX(CsSoundZde,                       KWavCsSoundZde),
   XX(CsSoundIhned,                     KWavCsSoundIhned),
   XX(CsSoundPo25Metrech,               KWavCsSoundPo25Metrech),
   XX(CsSoundPo50Metrech,               KWavCsSoundPo50Metrech),
   XX(CsSoundPo100Metrech,              KWavCsSoundPo100Metrech),
   XX(CsSoundPo200Metrech,              KWavCsSoundPo200Metrech),
   XX(CsSoundPo500Metrech,              KWavCsSoundPo500Metrech),
   XX(CsSoundPo25Yardech,               KWavCsSoundPo25Yardech),
   XX(CsSoundPo50Yardech,               KWavCsSoundPo50Yardech),
   XX(CsSoundPo100Yardech,              KWavCsSoundPo100Yardech),
   XX(CsSoundPo200Yardech,              KWavCsSoundPo200Yardech),
   XX(CsSoundPo500Yardech,              KWavCsSoundPo500Yardech),
   XX(CsSoundPo25Stopach,               KWavCsSoundPo25Stopach),
   XX(CsSoundPo50Stopach,               KWavCsSoundPo50Stopach),
   XX(CsSoundPo100Stopach,              KWavCsSoundPo100Stopach),
   XX(CsSoundPo200Stopach,              KWavCsSoundPo200Stopach),
   XX(CsSoundPo500Stopach,              KWavCsSoundPo500Stopach),
   XX(CsSoundPo1Kilometru,              KWavCsSoundPo1Kilometru),
   XX(CsSoundPo2Kilometrech,            KWavCsSoundPo2Kilometrech),
   XX(CsSoundPoPulMile,                 KWavCsSoundPoPulMile),
   XX(CsSoundPoCtvrtMile,               KWavCsSoundPoCtvrtMile),
   XX(CsSoundPoOsmineMile,              KWavCsSoundPoOsmineMile),
   XX(CsSoundPo1Mili,                   KWavCsSoundPo1Mili),
   XX(CsSoundPo2Milich,                 KWavCsSoundPo2Milich),
   XX(CsSoundAPotom,                    KWavCsSoundAPotom),
   XX(CsSoundCilJePredVami,             KWavCsSoundCilJePredVami),
   XX(CsSoundDrzteSeVlevo,              KWavCsSoundDrzteSeVlevo),
   XX(CsSoundDrzteSeVpravo,             KWavCsSoundDrzteSeVpravo),
   XX(CsSoundJedte,                     KWavCsSoundJedte),
   XX(CsSoundJedtePrimo,                KWavCsSoundJedtePrimo),
   XX(CsSoundJsteVCili,                 KWavCsSoundJsteVCili),
   XX(CsSoundNaKruhovemObjezdu,         KWavCsSoundNaKruhovemObjezdu),
   XX(CsSoundNebezpeci,                 KWavCsSoundNebezpeci),
   XX(CsSoundNyniJedeteNespravnymSmerem,KWavCsSoundNyniJedeteNespravnymSmerem),
   XX(CsSoundNyniJsteMimoTrasu,         KWavCsSoundNyniJsteMimoTrasu),
   XX(CsSoundOdbocteVlevo,              KWavCsSoundOdbocteVlevo),
   XX(CsSoundOdbocteVpravo,             KWavCsSoundOdbocteVpravo),
   XX(CsSoundOpustteDalnici,            KWavCsSoundOpustteDalnici),
   XX(CsSoundOtocteSeAzJeToMozne,       KWavCsSoundOtocteSeAzJeToMozne),
   XX(CsSoundProsimOtocteSe,            KWavCsSoundProsimOtocteSe),
   XX(CsSoundRychlostniKamera,          KWavCsSoundRychlostniKamera),
   XX(CsSoundVCili,                     KWavCsSoundVCili),
   XX(CsSoundVyjedte,                   KWavCsSoundVyjedte),
   XX(CsSoundVyjezdem,                  KWavCsSoundVyjezdem),
   XX(CsSoundCamera,                     KWavCsSoundCamera),

   /* Slovenian */
   // The clips must be sorted in AudioClipEnum-order
   XX(SlSoundPrvi,                             KWavSlSoundPrvi),
   XX(SlSoundDrugi,                            KWavSlSoundDrugi),
   XX(SlSoundTretji,                           KWavSlSoundTretji),
   XX(SlSoundCetrti,                           KWavSlSoundCetrti),
   XX(SlSoundPeti,                             KWavSlSoundPeti),
   XX(SlSoundSesti,                            KWavSlSoundSesti),
   XX(SlSoundSedmi,                            KWavSlSoundSedmi),
   XX(SlSoundOsmi,                             KWavSlSoundOsmi),
   XX(SlSoundIzhod,                            KWavSlSoundIzhod),
   XX(SlSoundCez25Metrov,                      KWavSlSoundCez25Metrov),
   XX(SlSoundCez50Metrov,                      KWavSlSoundCez50Metrov),
   XX(SlSoundCez100Metrov,                     KWavSlSoundCez100Metrov),
   XX(SlSoundCez200Metrov,                     KWavSlSoundCez200Metrov),
   XX(SlSoundCez500Metrov,                     KWavSlSoundCez500Metrov),
   XX(SlSoundCez1Kilometer,                    KWavSlSoundCez1Kilometer),
   XX(SlSoundCez2Kilometra,                    KWavSlSoundCez2Kilometra),
   XX(SlSoundInPotem,                          KWavSlSoundInPotem),
   XX(SlSoundIzberite,                         KWavSlSoundIzberite),
   XX(SlSoundKameriZaNadzorHitrosti,           KWavSlSoundKameriZaNadzorHitrosti),
   XX(SlSoundKoBoMogoceNareditePolkrozniZavoj, KWavSlSoundKoBoMogoceNareditePolkrozniZavoj),
   XX(SlSoundNaCilju,                          KWavSlSoundNaCilju),
   XX(SlSoundNaKroznemKriziscu,                KWavSlSoundNaKroznemKriziscu),
   XX(SlSoundNemudoma,                         KWavSlSoundNemudoma),
   XX(SlSoundOpozorilo,                        KWavSlSoundOpozorilo),
   XX(SlSoundOstaniteNaDesnemPasu,             KWavSlSoundOstaniteNaDesnemPasu),
   XX(SlSoundOstaniteNaLevemPasu,              KWavSlSoundOstaniteNaLevemPasu),
   XX(SlSoundPrisliSteNaSvojCilj,              KWavSlSoundPrisliSteNaSvojCilj),
   XX(SlSoundProsimoObrnite,                   KWavSlSoundProsimoObrnite),
   XX(SlSoundTukaj,                            KWavSlSoundTukaj),
   XX(SlSoundVasCiljJeNaravnost,               KWavSlSoundVasCiljJeNaravnost),
   XX(SlSoundZapeljiteIzAvtoceste,             KWavSlSoundZapeljiteIzAvtoceste),
   XX(SlSoundZapeljiteNaravnost,               KWavSlSoundZapeljiteNaravnost),
   XX(SlSoundZavijteDesno,                     KWavSlSoundZavijteDesno),
   XX(SlSoundZavijteLevo,                      KWavSlSoundZavijteLevo),
   XX(SlSoundZavijte,                          KWavSlSoundZavijte),
   XX(SlSoundZdajSteZasliSPoti,                KWavSlSoundZdajSteZasliSPoti),


};



const AudioClipTableTableDriven::int_to_clipname_t * OldAudioClipTable::getClipTable1()
{
   return m_staticClipTable1;
}

const AudioClipTableTableDriven::int_to_clipname_t * OldAudioClipTable::getClipTable2()
{
   return m_staticClipTable2;
}

int OldAudioClipTable::getNumClips1()
{
   return sizeof(m_staticClipTable1) / sizeof(m_staticClipTable1[0]);
}

int OldAudioClipTable::getNumClips2()
{
   return sizeof(m_staticClipTable2) / sizeof(m_staticClipTable2[0]);
}

