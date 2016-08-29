-----------------------
ithurricane [http://hi.baidu.com/ithurricane]
2010/02/04
-----------------------
support WDK7/VS2008
fix Privilege Problem
modify register source

-----------------------
bobo 2009/05/21
-----------------------
ÐÞ¸´bug: VC6 + DDK2003 -> vc6.dsp --> # PROP Cmd_Line "ddkbuild -WNETWXP free ."
         ÐÞ¸Ä: ÔÚddkbuild.cmdÖÐ,¹ØÓÚDDk2003Ã»ÓÐWNETWXP²ÎÊý£¬ÏÖÔÚÔö¼ÓWNETWXP²ÎÊý£¬Ð§¹ûµÈÍ¬ÓÚWNETXP

-----------------------
bobo 2009/05/08
-----------------------
exe:
  1.TestAppÓÉUnicode¸ÄÎªMulti-Byte(ÎªÁËÊ¡ÊÂ,×Ö·ûÆ´½Ó,×¢²á±úÒÁÐ´¶¼ÊÇMulti-Byte°æµÄ), 
      ¼´TestApp.vcprojµÄCharacterSet="1" ¸ÄÎªCharacterSet="2"
  2.TestApp¼ÓÔØºÍÐ¶ÔØsys(modify from MyZwDriverControl.c)£¬ Ã²ËÆ½â¾öÕâ¸öÎÊÌE
	  a.Æô¶¯app,load driver
	  b.°ÑappÇ¿ÖÆ¹ØµE´ËÊ±Ã»ÓÐÕý³£unload driver
	  c.ÔÙÆô¶¯app,´ËÊ±²»»áÔÙ´Îload driver(»òÕßËµÃ»ÓÐloadÊ§°Ü),Ò»ÇÐÕý³£Ê¹ÓÃdriver
	  d.Õý³£ÍË³öapp,unload driver

	  ps: MyZwDriverControl ÓEclass LoadNTDriver(copy from 0ginr.com)ÓÐÇø±ð´¦, 
	      MyZwDriverControlÔÚloadºÍunloadÄÚ,¶¼»á×ö×¢²á±úèúØÓ,É¾³ý¹¤×E
		  class LoadNTDriverµÄloadÄÚÌúØÓ×¢²á±EunloadÄÚÉ¾³ý×¢²á±E
		  MyZwDriverControl¿ÉÒÔLockXxxDatabase.
  3.¼ÓÔØºÍÐ¶ÔØsys¿ÉÒÔÓÃclass LoadNTDriver, ·½·¨¼ELoadNTDriver.h"

sys:
  DriverEntryÀEòÓ¡×¢²á±úÞ·¾¶
  default uIoControlCode Êä³EIoCode
  ËùÓÐº¯ÊýÔö¼Ó·ÖÒ³¡¢·Ç·ÖÒ³ÉùÃE#pragma alloc_text(PAGE, DispatchXX))

-----------------------
bobo 2009/04/27
-----------------------
1.¸÷¸ötarget os»¥³EÖ»ÄÜÑ¡Ò»¸öos
2.¸ù¾ÝDDK°æ±¾£¬ÏÞÖÆOSÑ¡ÔE
3.fix bug : "mybuild.bat" ÄÚµÄ PROJECT_MYBUILD_CMD_LINE ²ÎÊý¸ù¾Ý(DDK + OS)Ð´ËÀ£¬
            DDKÂ·¾¶¶¼ÊÇÄ¬ÈÏµÄ£¬ÔÙ¸ã¸ö¶ÁÐ´Âé·³¸ö¡£

			DDK 2000       --- C:\\NTDDK
			DDK XP         --- C:\\WINDDK\\2600
			DDK 2003       --- C:\\WINDDK\\3790.1830
			WDK 6001.18002 --- C:\\WINDDK\\6001.18002

4.Ö§³ÖWDK + win2008
// todo : 5.Ôö¼Ócheck£¬ freeÑ¡ÔE(mybuild.bat½Å±¾ÀEàÒEÎÊý "chk ." ÓE"fre .")

6.µÚÒ»´ÎÆô¶¯EasySysÊ±£¬»á×Ô¶¯Éè¶¨ÓÃ»§»·¾³±äÁ¿(WXPBASE,WNETBASE...)£¬
  ÕâÊÇ¸ù¾Ý×¢²á±EHKEY_LOCAL_MACHINE -- SOFTWARE\\Microsoft\\WINDDK Éè¶¨DDKÂ·¾¶µÄ¡£
  (ÍòÒ»ÓÃ»§°²×°ÁË¶à¸öDDK£¬ÔÙ¼Ó¸öWDK£¬Õâ¸öÂ·¾¶¾ÍÒªÁúéâÅÐ¶ÏÁË)

  µ«"mybuild.bat"Ò²»áset»·¾³±äÁ¿£¬ËùÒÔÕâÀEäÊµÃ»ÓÃÁË¡£

7.½Å±¾ÃEûÎÎÊý¾ßÌå¿´ "ddkbuild.cmd":: Usage output

-----------------------
bobo 2009/04/23
-----------------------
1.²ÎÕÕMS sample¸ñÊ½ÐÞ¸Ä"sources"

-----------------------.
bobo 2009/04/18
-----------------------
1. Ôö¼Ócommon.h, ÓÃÓÚ¶¨Òåexe¡¢sysµÄ¹²Í¨IOCTL
2. Ôö¼Óconsole APP (for VS2005)¹¤³Ì, ÓÃÓÚ²âÊÔsys. ´ÓÄÄ¸öhello world°áÀ´µÄ.
   (ÕâÑùÖ»ÒªÅäºÏDriverMonitor¾Í¿ÉÒÔ²âÊÔÇý¶¯ÁË)
// todo : 3. ÔÚdsp_proj.htm(VC6)ÀEö¼Óclean±àÒEÎÊý,ÈÃVC6Ê¹ÓÃclean.bat
// todo : 4. Ôö¼ÓMFC APP¹¤³Ì,ÓÃÓÚ²âÊÔsys
5. ÌúØÓÐÞ¸Ä×¢ÊÍ
6.ÐÞ¸Äprintºê£¬¶ÔÓ¦checkºÍfree°æ±¾µÄbuild
  #if DBG
  #define dprintf DbgPrint
  #else
  #define dprintf
  #endif

Ô½Ð´Ô½¸´ÔÓÁË£¬²»easyÁË.....

-----------------------
bobo 2009/04/16
-----------------------
1.OnInitDialog()ÀEö¼ÓOnChangeAuthor()
2.ÐÞ¸ÄOnChangeAuthor()×Ö·û´®
3.ÐÞ¸Ähfile.htm,cfile.htmÀ´ÊÊÓ¦×Ô¼ºÏ°¹ß
-----------------------

bobo 2009/04/10
-----------------------
1.GenerateRandomStrings()µÄnew Ã»ÓÐÊÍ·Å£¬
  ¸ÄÎª¾Ö²¿Êý×EÃû×Ö²»Ì«³¤¾ÍÐÐ,64´óÐ¡)
2.OnInitDialog()ÀEö¼ÓOnSelectIdeType(),³õÊ¼»¯IdeType
				  Ôö¼ÓOnSelchangeDdkVersion(),³õÊ¼»¯DDK type
3.ddkbuild.cmd ¸EÂÎª VERSION=V7.3
4.VC6µÄddkbuild.bat¸ÄÎªddkbuild.cmd
5.ÐÞ¸ÄOnSelchangeDdkVersion()ÄÚµÄDDK2000µÄÂß¼­ÅÐ¶Ï
6.ÔÚVC6µÄdsp.htmÖÐÔö¼Óstruct.h

-----------------------
ChiChou [http://hi.baidu.com/517826104]
2009/02/03
-----------------------

ChiChou ¸ù¾Ý sudamiÐÞ¸Ä°EÖÆ×E

* ÐÞÕýBUG
	1.·Ç³£ÑÏÖØµÄBUG¡­¡­³ÌÐòÍË³öÊ±Ã»ÓÐÒÆ³ýÍÐÅÌÍ¼±ê£¬ÔÚÄ³Ð©»úÆ÷ÉÏÍË³öÊ±
»á±ÀÀ££¨ÖÁÉÙÎÒµÄµçÄÔÊÇÕâÑE- -!£©¡£

	2.Ô­°æ»ñµÃDDKÂ·¾¶Ê±£¬¶ÁÈ¡µÄ×¢²á±úÞ·¾¶Ð´´úÝË£¬µ¼ÖÂÎÞ·¨»ñµÃ DDK µÄ 
Path£¨Ã²ËÆ´óÃ×Ö±½ÓÌûåÏ×Ô¼ÒµÄDDKÂ·¾¶ÁËÖ®¡£¡£Î´ÃâÓÐµã¡£¡£¡££©¡£

	3.Ô­°æÃ»ÓÐ¼Eé¹¤³ÌÃû£¬Èç¹ûÀEæ°E¬·Ç·¨×Ö·û£¬ÄÇÃ´´´½¨¹¤³ÌÄ¿Â¼µÄÊ±ºE
¾Í»á³ö´ú½£ÐÞÕýÊ±Ôö¼Ó¶Ô·Ç·¨×Ö·ûµÄcheck¹¦ÄÜ¡£°E¨£º/\|"<>*?:

	4.»¹ÓÐÒ»¸öBUGÃ»ÓÐÐÞ¸´¡£Èç¹ûÔÚ¹¤³ÌÃûÖÐÌûäEÎÈç¡°project..¡±µÄÃû³ÆÊ±£¬
Ò²»áÒý·¢ÎÄ¼þ·ÃÎÊ´úêó¡£µ«ÊÇÎÒÃ»ÓÐ°E¨ÐÞ¸´¡­¡­Õâ¸ö×Ö·û´®µÄÅÐ¶ÏÓÐµãÄÑ¶È¡£¡£
Ë®Æ½µÍÁÓ¡£¡£(- -...)

* ÃÀ»¯½çÃE
	1.°Ñ´óÃ×ÅªµÄ XPBUTTON ¸øÈ¥µôÁË...Í¼±àÒEÙ¶È¡£
	2.È»ºóPÁËÕÅ±³¾°Í¼£¬·ÅÔÚÖ÷½çÃæ£¬»¹ÂùºÃ¿´µÄ~~
	3.Ï°¹ßÐÔµÄ XP Manifest ´¦ÀúëÂ¡£

* ÐÞ¸ÄÄ£°E
	1.¸ù¾Ý¸öÈËÏ²ºÃ´úÂEÄ¸ñÊ½ÐÞ¸ÄÁËÏÂ~
	2.Ë³±ã°Ñ¡¶ÌEéÒ¹¶Á¡·ÀEæ¿´µ½µÄÄ³¸ö¶«Î÷£¨ÅÐ¶Ï·ûºÅÁ´½ÓÓÃ»§Ïà¹ØÐÔÈ»ºE
´´½¨²»Í¬µÄÉè±¸·ûºÅÃû£©¼ÓÁË½øÈ¥¡­¡­

* ÍEÆ¹¦ÄÜ
	1.sudami°æµÄÓÐµãÂé·³£¬ÍË³ö³ÌÐò»¹ÒªµãÍÐÅÌ¡£Ö±½ÓÔÚÖ÷½çÃæÉèÖÃÁËÍË³ö°´
Å¥¡£
	2.¹¤³Ì´´½¨Ö®ºóEasySYS»ù±¾Ã»ÓÃ´¦ÁË£¬±ãÔö¼ÓÁË¡°ÍEÉºóÍË³ö¡±Ñ¡Ïû›
	3.Éú³É¹¤³Ìºó¿ÉÒÔÔ´ÂEÐÌúØÓÈÕÆÚ¡¢×÷ÕßµÈÐÅÏ¢¡£
	4.Éú³É¹¤³ÌºóµÄ²Ù×÷Ò²ÊÇ¿ÉÑ¡µÄ¡£¿ÉÒÔÑ¡Ôñ×Ô¶¯´ò¿ª¹¤³ÌÎÄ¼þ¡¢¹¤³ÌÄ¿Â¼¡£
	5.Ôö¼ÓClean¹¦ÄÜ~~~
-----------------------


-----------------------
sudami [sudami@163.com]
2008/08/13
-----------------------

	VS 2005ÏÂÐ´Çý¶¯±ÈVC 6.0·½±ãºÜ¶Eµ«ºÃ¶àEasySYS°æ±¾(Include DIY)¶¼Ö»
Ö§³ÖVC 6.0.½ñ¶ùÔçÉÏÔÚVBGOODÂÛÌ³¿´µ½IceBoyÐ¡Í¬Ñ§ÓÃVBÐ´ÁË¸öÖ§³Övs 2005µÄ
demo.ÓÚÊÇ,°³Ò²ÊÔ×ÅÐ´¸öÊÊÓÃÓÚ×Ô¼ºµÄ¹¤¾ß,·½±ãÒÔºó¿EÙµÄ¿ª·¢Çý¶¯³ÌÐE

	¹þ¹þ,Ö÷ÒªÊÇ·½±ã×Ô¼ºÐ´code,¹ÊÉú³ÉµÄcode·ç¸ñ¶¼ÊÇ°´ÕÕ×Ô¼ºµÄÒâÔ¸À´°²ÅÅ
µÎ...

-----------------------
