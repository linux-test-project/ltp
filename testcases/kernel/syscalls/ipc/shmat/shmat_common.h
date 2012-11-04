static key_t probe_key;

void *probe_free_addr(void)
{
	void *p;
	int ret;
	int shm_id = -1;

	if (probe_key == 0)
		probe_key = getipckey();

	/* create a shared memory resource with read and write permissions
	 * We align this to SHMLBA so we should allocate at least
	 * SHMLBA*2 in case SHMLBA > page_size. */
	shm_id = shmget(probe_key, SHMLBA*2, SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id == -1)
		tst_brkm(TBROK, cleanup, "probe: shmget failed");

	/* Probe an available linear address for attachment */
	p = shmat(shm_id, NULL, 0);
	if (p == (void *)-1)
		tst_brkm(TBROK, cleanup, "probe: shmat failed");
	ret = shmdt(p);
	if (ret == -1)
		tst_brkm(TBROK, cleanup, "probe: shmdt failed");

	rm_shm(shm_id);

	/* some architectures (e.g. parisc) are strange, so better always
	 * align to next SHMLBA address. */
	p = (void *)(((unsigned long)(p) + (SHMLBA - 1)) & ~(SHMLBA - 1));
	return p;
}
