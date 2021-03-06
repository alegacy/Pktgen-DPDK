/*-
 * <COPYRIGHT_TAG>
 */
/**
 * Copyright (c) <2010-2012>, Wind River Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed in the application must contain the copyright notice as
 * defined above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _WR_MBUF_H_
#define _WR_MBUF_H_

#ifdef __cplusplus
extern "C" {
#endif

static inline struct rte_mbuf *rte_pktmbuf_alloc_noreset(struct rte_mempool *mp)
{
	struct rte_mbuf *m;
	if ((m = __rte_mbuf_raw_alloc(mp)) != NULL) {
		m->pkt.nb_segs = 1;
		m->pkt.next = NULL;
	}
	return (m);
}

static inline int rte_pktmbuf_alloc_bulk_noreset(struct rte_mempool *mp, void ** obj_table, unsigned cnt)
{
	int		ret;
	unsigned i;
	struct rte_mbuf * m;

	ret = rte_mempool_get_bulk(mp, obj_table, cnt);

	if ( ret == 0 ) {
		for(i=0; i<cnt; i++) {
			m = (struct rte_mbuf *)obj_table[i];
			RTE_MBUF_ASSERT(rte_mbuf_refcnt_read(m) == 0);
			rte_mbuf_refcnt_set(m, 1);
			m->pkt.nb_segs = 1;
			m->pkt.next	= NULL;				// Must set the next pointer to null.
		}
		return cnt;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* _WR_MBUF_H_ */
