/************************************************************************
 *                                                                      *
 * awebshell.rexx */                                                    *
 *                                                                      *
 * 2007-07-10 - Added the missing 'return e' in errortext: that somehow *
 * vanished from the last update(?), thanks again Frank <sbowman>       *
 *                                                                      *
 * 2007-07-09 - Fixed commands.html link, thanks Frank Weber <sbowman>  *
 *                                                                      *
 * Allow the user to control AWeb by keyboard commands typed in a       *
 * shell window.                                                        *
 *                                                                      *
 * Usage:                                                               *
 *    rx AWebShell.rexx [screenname]                                    *
 *                                                                      *
 * By default the shell window opens on the current AWeb screen.        *
 *                                                                      *
 * Call this macro from AWeb (menu: ARexx/Start ARexx macro), or        *
 * configure it in the ARexx menu.                                      *
 *                                                                      *
 ************************************************************************/

options results

arg s_screen

s_port=address()
if left(s_port,5)='AWEB.' then do
   s_title='from' s_port
end
else do
   s_port='AWEB.NOPORT'
   call checkport
   s_title=''
end

address value s_port
if s_screen='' then do
   'get screen'
   s_screen=result
end

if ~open(s_con,'con:999/999/512/200/AWeb shell' s_title'/close/screen' s_screen) then exit
call writeln s_con,'AWeb shell. Type -h for help.'

options failat 21

do forever
   call checkport
   call writech s_con,s_port'> '
   s_inline=readln(s_con)
   if eof(s_con) then exit
   parse upper var s_inline s_kwd s_line
   select
      when s_kwd='-A' then do
         parse upper var s_line s_nr .
         s_newport='AWEB.'s_nr
         if find(show('P'),s_newport)>0 then do
            s_port=s_newport
         end
         else call writeln s_con,'Port "'s_newport'" not found.'
      end
      when s_kwd='-D' then do
         call checkport
         address value s_port
         'iconify show'
         'open file://localhost/AWebPath:Docs/Advanced/ARexx/commands.html target AWebShellHelp'
         'get port target AWebShellHelp'
         if rc=0 then do
            address value result
            'window tofront'
         end
      end
      when s_kwd='-H' then do
         call writeln s_con,''
         call writeln s_con,'Type AWeb ARexx commands, or any of the following'
         call writeln s_con,'escape commands:'
         call writeln s_con,''
         call writeln s_con,'-A number         address other AWeb port'
         call writeln s_con,'-D                show ARexx documentation'
         call writeln s_con,'-H                show this help information'
         call writeln s_con,'-S stem name ...  show all STEM.#.NAME [#=1..STEM.0]'
         call writeln s_con,'-V var            show variable VAR'
         call writeln s_con,'-X                exit shell'
         call writeln s_con,''
         call writeln s_con,'Note: In VAR and STEM arguments, do not use variable'
         call writeln s_con,'names starting with "S_"'
         call writeln s_con,''
      end
      when s_kwd='-S' then do
         parse upper var s_line s_stem s_line
         interpret 's_v=symbol("'s_stem'.0")'
         if s_v='VAR' then do
            s_nnames=0
            do s_i=1 while s_line~=''
               parse var s_line s_names.s_i s_line
               s_nnames=s_i
            end
            interpret 's_v='s_stem'.0'
            call writeln s_con,s_v 'results:'
            do s_j=1 to s_v
               do s_i=1 to s_nnames
                  interpret 's_v='s_stem'.'s_j'.'s_names.s_i
                  call writeln s_con,s_stem'.'s_j'.'s_names.s_i'="'s_v'" '
               end
            end
         end
         else call writeln s_con,'Stem variable "'s_stem'" not found.'
      end
      when s_kwd='-V' then do
         parse upper var s_line s_name .
         interpret 's_v='s_name
         call writeln s_con,s_name'="'s_v'"'
      end
      when s_kwd='-X' then exit
      when s_inline~='' then do
         drop result
         call checkport
         address value s_port
         ''s_inline
         if rc>1 then do
            call writeln s_con,'ERROR:' errortext(rc)
         end
         else do
            if symbol('RESULT')='VAR' then do
               call writeln s_con,'RESULT="'result'"'
            end
         end
      end
      otherwise
   end
end

checkport: procedure expose s_port
   ports=show('P')
   if find(ports,s_port)=0 then do
      nr=''
      parse var ports dummy 'AWEB.' nr .
      if nr='' then exit
      address value 'AWEB.'nr
      'get activeport'
      s_port=result
   end
return

errortext: procedure
   parse arg code
   select
      when code=5 then e='Command could not be completed'
      when code=10 then e='Invalid arguments'
      when code=11 then e='Invalid command'
      when code=20 then e='Internal error'
      otherwise e='Unknown return code:' code
   end
return e
