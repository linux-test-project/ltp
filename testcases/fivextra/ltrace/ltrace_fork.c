main()
{
	if (fork() == 0) {
		write(1, "child\n", 6);
		sleep(10);
	}
	else {
		wait(0);
		write(1, "parent\n", 7);
		sleep(10);
	}
	return(0);
}
