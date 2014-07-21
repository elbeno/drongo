#ifndef _PARSE_H_
#define _PARSE_H_ 1

/* splitParse(text, err) - binds together the input arguments,
   splitting either side of text 'text' - error message 'err'
   is displayed in an error..
   e.g. splitParse("from", "steal <what> from <whom>?"); would
   return the <what> in $first, and <whom> in $second */

#define splitParse(txt, err) $first=""; $second=""; $spc=""; $tmpscan=1;\
  foreacharg $p {\
    if ($tmpscan)\
      if ($p==txt) { $spc=""; $tmpscan=0; }\
      else { $first=str($first,$spc,$p); $spc=" "; }\
    else { $second=str($second,$spc,$p); $spc=" "; }}\
  if (($tmpscan==1) || ($first=="") || ($second=="")) {\
    echo(err,"\n"); return; }

#define bundleArgsNLs $arg="  "; $spc=""; \
  foreacharg $tmparg { if ($tmparg=="*n") \
                         $arg=str($arg, "\n ");\
                       else { $arg=str($arg, $spc, $tmparg);\
                         $spc=" "; }}

#endif
