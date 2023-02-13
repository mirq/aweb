/* Script to update all .ct files after modifying .cd file */


options results

parse arg descriptor .

catalog_path     = "Projects:AWeb/awebHEAD/"
translation_path = "Projects:AWeb/awebHEAD/catalogs/~(#?CVS#?)"

list_cmd = "C:list  lformat = %p%s/" || descriptor || ".ct DIRS TO T:update_ct_temp " || translation_path

address command list_cmd

if open('IN',"T:update_ct_temp",'R') then do
    ctfile = readln('IN')
    do while ~eof('IN')
        flexcat_cmd = "flexcat " || descriptor || ".cd " || ctfile || " NEWCTFILE T:temp.ct COPYMSGNEW"
        copy_cmd = "copy T:temp.ct " || ctfile
        delete_cmd = "delete T:temp.ct"

        address command flexcat_cmd
        address command copy_cmd
        address command delete_cmd
        ctfile = readln('IN')
    end
    call close('IN')
    address command "C:delete T:update_ct_temp"
end


