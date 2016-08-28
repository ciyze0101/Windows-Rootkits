rem /////////////////
rem / Add by ChiChou
rem / 
rem / FileName:Clean.bat
rem / Description:Clean
rem /
rem ////////////////
rd .\bin /s /q
rd .\WinDDK /s /q
rd .\objchk_w2k_x86 /s /q
rd .\objchk_wxp_x86 /s /q
rd .\objchk_wnet_x86 /s /q
rd .\objchk_wlh_x86 /s /q
rd .\objfre_w2k_x86 /s /q
rd .\objfre_wxp_x86 /s /q
rd .\objfre_wnet_x86 /s /q
rd .\objfre_wlh_x86 /s /q
del .\*.log
del .\*.err
del .\*.xml
rem ***** del VS2005 file *****
del .\*.ncb
del .\*.user
del .\*.suo /A:H
rem ***** del VS6.0 file *****
del .\*.plg
del .\*.opt
exit