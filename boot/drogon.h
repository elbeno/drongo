/*  Drogon.h  */

/* Revisions:
Date     | Vsn  |
---------+------+------------------------------------------------------------
22/04/94 | 1.01 | Copied from rws13 to fix becomePlayer bug and add a couple
	 |	| of macros
	 |	| Added WETASK macro - set task level number for object
26/04/94 | 1.02 | Updated QPLAYER for strict Drogon compatibility  
09/05/94 | 1.03 | Added MTEXT and creatorCommand 
13/05/94 | 1.04 | Changed call to newRoom from newRoomNo (boot.u v.1.16)
16/05/94 | 1.05 | In v.1.04 rev comment, changed 2nd to to from cos someone
		  was being picky.
*/

#ifndef _DROGON_H_
#define _DROGON_H_ 1

#define ROOM(NAME,URN) $room = @._newRoom(NAME,URN) ; \
		echo("#define ",URN," ",$room,"\n") ;
#define SEA_ROOM(NAME,URN) ROOM(NAME,URN)
#define RDESC $room.description
#define HAVEN $room.haven = 1 ;
#define HAVEN_TEXT $room.havenTxt 
#define NO_SUMMON_IN $room.noSummonIn = 22 ;
#define NO_SUMMON_OUT $room.noSummonOut = 22 ;
#define NO_TP_IN $room.noTpIn = 22 ;
#define NO_TP_OUT $room.noTpOut = 22 ;
/* These need to be checked and supported: */
#define NO_HOME $room.noHome = 22 ;
#define WIZ_AREA NO_HOME ; NO_TP_IN ; NO_TP_OUT ; NO_SUMMON_IN ; NO_SUMMON_OUT ;

#define RXFETCH $room.rxfetch = 22 ;
#define INDOORS $room.indoors = 1 ;

#define BI_CONNECT(RM1,RM2,DR1,DR2) @._link (RM1,RM2,DR1) ; \
			 @._link (RM2,RM1,DR2) ;
#define CONNECT(RM1,RM2,DIR) @._link (RM1,RM2,DIR) ;

#define SCENERY(OBJ,SCN) \
        $scene = @._addScenery ($room, OBJ, SCN) ;
#define SDESC $scene.description 

#define PLAYER(NAME) $player = @._newPlayer (NAME) ; \
		echo ("#define ",NAME," ",$player,"\n") ;
#define MALE 1
#define FEMALE 0
#define NEUTER -1
#define PDESC $player.description

#define WIDGET(NAME,UWN,VAL,WGT,WEA,ARM,SLP,LOC) \
	$widget = @._newWidget(NAME,UWN,VAL,WGT,WEA,ARM,SLP,LOC) ; \
	echo("#define ",UWN," ",$widget,"\n") ;

#define WDESC $widget.description 
#define WDIEDROP $widget.diedrop = 1 ;
#define WEDIBLE $widget.edible = 1 ;
#define WDRINKABLE $widget.drinkable = 1 ;
#define WXFETCH $widget.xFetch = 1
#define WSTICKY $widget.sticky = 1
#define WINVISIBLE $widget.invis = 1 ;
#define WSPECIAL $widget.special = 1 ;
#define WTEXT $widget.text
#define WDNAME $widget.dName
#define WFIXED $widget.fixed 
#define WCLOAK $widget.cloakingDevice = 1 ;
#define WETASK $widget.taskEnd 

#define MOBILE(NAME,UMN,SEX,VAL,HP,T,LOC) \
	$mobile = @._newMobile(NAME,UMN,SEX,VAL,HP,T,LOC) ; \
	echo("#define ",UMN," ",$mobile,"\n") ;
#define MDESC $mobile.description 
#define MTEXT $mobile.text 
#define MFIXED $mobile.fixed 
#define MDNAME $mobile.dName
#define MATTACK $mobile.attack
#define MXATTACK $mobile.mxattack
#define MENTER $mobile.enterTxt
#define MEXIT $mobile.exitTxt
#define MOBILE_MAP $mobile.map = listnew
#define MWEAPON $mobile.weapon = $widget ;
#define MMOVEPROB $mobile.moveprob
#define MATTACKPK $mobile.attackpk = 1;

#define WIZ(p) ( p.level >= 20 ) 
#define Y_WIZ(p) ( p.level >= 20 )
#define M_WIZ(p) ( p.level >= 21 )
#define S_WIZ(p) ( p.level >= 22 )
#define ARCH(p) ( p.level >= 25 ) 
#define CREATOR(p) ( p.level >= 29 ) 
#define creatorCommand if (!CREATOR(#actor)) {echo (@.Huh);return; }
#define archWizCommand if (!ARCH(#actor)) { echo (@.Huh);return; }
#define mWizardCommand if (!M_WIZ(#actor)) { echo (@.Huh);return; }
#define wizardCommand if (!WIZ(#actor)) { echo (@.Huh);return; }

#define swirlingVoid #2
#define specialStore #4
#define generalStore #3
#define Util #6
#define CodeSub #8
#define bareHandsItem #9

#define QPLAYER(x) (x.player != NULL)

#define DRONGO 1
#define Mins(N) N*60

#define becomePlayer(WHO) $me = #actor ; setruid (WHO) ;
#define becomeMyself setruid ($me) ;

#define minArgs(N,WARN) if ($# < N) { echo (WARN,"\n") ; return ; }
#define numArgs(N,WARN) if ($# != N) { echo (WARN,"\n") ; return ; }
#define bundleArgs $arg = ""; $spc = "" ; \
  foreacharg $tmparg { $arg = str($arg, $spc, $tmparg) ; $spc = " " ; }
#define bundleArgsNoSpace $arg = "";  \
	 foreacharg $tmparg { $arg = str($arg, $tmparg) ; }

@._setArea (str("default",atoobj(#actor))) ;

echo ("Start of compile\n");

#endif

