i=1
while read -r line; do
	utterId=${line:0:18}
	wavpath=${line:19}
	speaker=${utterId:0:12}
	cmvn=`grep $speaker cmvn.scp`
	feats=`grep $utterId feats.scp`
	utt2spk=`grep $utterId utt2spk`
	spk2utt=`grep $utterId spk2utt`
	text=`grep $utterId text`
	mkdir $i
	echo $cmvn > $i/cmvn.scp
	echo $feats > $i/feats.scp
	echo $utt2spk > $i/utt2spk
	echo $spk2utt > $i/spk2utt
	echo $text > $i/text
	echo $line > $i/wav.scp
	i=$((i+1))
	echo $i
done < wav.scp
