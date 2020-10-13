#!/bin/bash
# man pages must be installed
for i in oggCat oggCut oggJoin oggSplit oggTranscode oggSlideshow oggSilence oggThumb oggDump mkThumbs oggLength
do
  echo "creating $i.html"
  man2html -f $i.1 > $i.html
done
