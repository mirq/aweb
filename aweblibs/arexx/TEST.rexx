/**/

ADDRESS AWEB.1

NL = X2c("0A")

GETCFG HTMLMODE VAR FOO

SAY FOO

S.0 = 2

S.1.ITEM = HTMLMODE
S.1.VALUE = 2

s.2.ITEM = HOTLISTOPEN
S.2.VALUE = 0


SETCFG STEM S

GETCFG HTMLMODE VAR FOO

SAY FOO

BAR.0 = 2
BAR.1.GROUP = 0
BAR.1.TITLE = "Test Entry One"
BAR.1.URL   = "http://foo.com"
BAR.1.OWNER = 0
BAR.2.GROUP = 0
BAR.2.TITLE = "Test Entry Two"
BAR.2.URL   = "ftp://bar.com"
BAR.2.OWNER = 0

HOTLIST SET BAR

GET HOTLIST STEM 'FOO'

DO I = 1 TO FOO.0
SAY FOO.I.TITLE
SAY FOO.i.url
END

HOTLIST RESTORE

GET HOTLIST STEM 'FOO'

DO I = 1 TO FOO.0
SAY FOO.I.TITLE
SAY FOO.i.url
END

BISCUIT.1.NAME="FOOD"
BISCUIT.1.VALUE="MILK"
BISCUIT.1.DOMIAN="bar.com"
BISCUIT.1.PATH="/kitchen"
BISCUIT.1.EXPIRES = "00:00:00 1-JAN-2009"


BISCUIT.0 = 1

SETCOOKIES STEM BISCUIT ADD

GET COOKIES STEM CK

do i = 1 to ck.0
say ck.i.name
say ck.i.domain
end
