int i, byTwo, byThree;

i = 0;
byTwo = 2;
byThree = 3;

while (i < 10) {
	if (i / 2 == 1) {
		puts byTwo;
	} else {
		if (i / 3 == 1) {
			puts byThree;
		}
	}

	i = i + 1;
}

