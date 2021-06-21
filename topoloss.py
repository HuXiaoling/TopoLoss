# Tips to incorporate the topological loss into your own framework
# You have to pretrain your model first and then fine-tune with topoloss, see the rationale part of our paper
# train(pretrain_epoch, topo_epoch):
#    for i in range(0, topo_epoch):
#       if (i<= pretrain_epoch):
#         loss = nn.CrossEntropyLoss();
#       else:
#         loss = nn.CrossEntropyLoss() + lambda * getTopoLoss(lh, gt);


import time
import numpy
import gudhi as gd
from pylab import *
import torch

t0=time.time();

def compute_dgm_force(lh_dgm, gt_dgm, pers_thresh=0, pers_thresh_perfect=0.99, do_return_perfect=False):

    lh_pers = abs(lh_dgm[:, 1] - lh_dgm[:, 0])
    if (gt_dgm.shape[0] == 0):
        gt_pers = None;
        gt_n_holes = 0;
    else:
        gt_pers = gt_dgm[:, 1] - gt_dgm[:, 0]
        gt_n_holes = gt_pers.size  # number of holes in gt
        # gt_n_holes = (gt_pers > 0.03).sum()  # number of holes in gt # ignore flat ones

    if (gt_pers is None or gt_n_holes == 0):
        idx_holes_to_fix = list();
        idx_holes_to_remove = list(set(range(lh_pers.size)))
        idx_holes_perfect = list();
    else:
        # more lh dots than gt dots

        # print (lh_pers.shape)
        # print(lh_pers.size)
        # print (gt_pers.shape)
        # assert lh_pers.size > gt_pers.size
        assert lh_pers.size >= gt_n_holes
        if (lh_pers.size < gt_n_holes):
            gt_n_holes = lh_pers.size

        # check to ensure that all gt dots have persistence 1
        tmp = gt_pers > pers_thresh_perfect

        # assert tmp.sum() == gt_pers.size

        # get "perfect holes" - holes which do not need to be fixed, i.e., find top
        # lh_n_holes_perfect indices
        # check to ensure that at least one dot has persistence 1; it is the hole
        # formed by the padded boundary
        # if no hole is ~1 (ie >.999) then just take all holes with max values
        tmp = lh_pers > pers_thresh_perfect  # old: assert tmp.sum() >= 1
        # print('pers_thresh_perfect', pers_thresh_perfect)
        # print('lh_pers > pers_thresh_perfect', (lh_pers > pers_thresh_perfect).sum())
        # print (type(tmp))
        lh_pers_sorted_indices = np.argsort(lh_pers)[::-1]
        if np.sum(tmp) >= 1:
            # if tmp.sum >= 1:
            # n_holes_to_fix = gt_n_holes - lh_n_holes_perfect
            lh_n_holes_perfect = tmp.sum()
            # idx_holes_perfect = np.argpartition(lh_pers, -lh_n_holes_perfect)[
            #                    -lh_n_holes_perfect:]
            idx_holes_perfect = lh_pers_sorted_indices[:lh_n_holes_perfect];
        else:
            # idx_holes_perfect = np.where(lh_pers == lh_pers.max())[0]
            idx_holes_perfect = list();

        # find top gt_n_holes indices
        # idx_holes_to_fix_or_perfect = np.argpartition(lh_pers, -gt_n_holes)[
        #                              -gt_n_holes:]
        idx_holes_to_fix_or_perfect = lh_pers_sorted_indices[:gt_n_holes];

        # the difference is holes to be fixed to perfect
        idx_holes_to_fix = list(
            set(idx_holes_to_fix_or_perfect) - set(idx_holes_perfect))

        # remaining holes are all to be removed
        # idx_holes_to_remove = list(
        #    set(range(lh_pers.size)) - set(idx_holes_to_fix_or_perfect))
        idx_holes_to_remove = lh_pers_sorted_indices[gt_n_holes:];

    # only select the ones whose persistence is large enough
    # set a threshold to remove meaningless persistence dots
    # TODO values below this are small dents so dont fix them; tune this value?
    pers_thd = pers_thresh
    idx_valid = np.where(lh_pers > pers_thd)[0]
    idx_holes_to_remove = list(
        set(idx_holes_to_remove).intersection(set(idx_valid)))

    force_list = np.zeros(lh_dgm.shape)
    # push each hole-to-fix to (0,1)
    force_list[idx_holes_to_fix, 0] = 0 - lh_dgm[idx_holes_to_fix, 0]
    force_list[idx_holes_to_fix, 1] = 1 - lh_dgm[idx_holes_to_fix, 1]

    # push each hole-to-remove to (0,1)
    force_list[idx_holes_to_remove, 0] = lh_pers[idx_holes_to_remove] / \
                                         math.sqrt(2.0)
    force_list[idx_holes_to_remove, 1] = -lh_pers[idx_holes_to_remove] / \
                                         math.sqrt(2.0)

    if (do_return_perfect):
        return force_list, idx_holes_to_fix, idx_holes_to_remove, idx_holes_perfect

    return force_list, idx_holes_to_fix, idx_holes_to_remove

def getCriticalPoints(likelihood):

    lh = 1 - likelihood
    lh_vector = np.asarray(lh).flatten()

    lh_cubic = gd.CubicalComplex(
        dimensions=[lh.shape[0], lh.shape[1]],
        top_dimensional_cells=lh_vector
    )

    Diag_lh = lh_cubic.persistence(homology_coeff_field=2, min_persistence=0)
    pairs_lh = lh_cubic.cofaces_of_persistence_pairs()

    # if(torch.min(lh_patch) == 1 or torch.max(lh_patch) == 0): continue
    # if(torch.min(gt_patch) == 1 or torch.max(gt_patch) == 0): continue
    
    # return persistence diagram, birth/death critical points
    pd_lh = numpy.array([[lh_vector[pairs_lh[0][0][i][0]], lh_vector[pairs_lh[0][0][i][1]]] for i in range(len(pairs_lh[0][0]))])
    bcp_lh = numpy.array([[pairs_lh[0][0][i][0]//lh.shape[1], pairs_lh[0][0][i][0]%lh.shape[1]] for i in range(len(pairs_lh[0][0]))])
    dcp_lh = numpy.array([[pairs_lh[0][0][i][1]//lh.shape[1], pairs_lh[0][0][i][1]%lh.shape[1]] for i in range(len(pairs_lh[0][0]))])

    return pd_lh, bcp_lh, dcp_lh

def getTopoLoss(likelihood, gt):
    # topo_size = likelihood.shape[0]
    topo_size = 200
    topo_cp_weight_map = np.zeros(likelihood.shape)
    topo_cp_ref_map = np.zeros(likelihood.shape)

    for y in range(0, likelihood.shape[0], topo_size):
        for x in range(0, likelihood.shape[1], topo_size):

            lh_patch = likelihood[y:min(y + topo_size, likelihood.shape[0]),
                         x:min(x + topo_size, likelihood.shape[1])]
            gt_patch = gt[y:min(y + topo_size, gt.shape[0]),
                         x:min(x + topo_size, gt.shape[1])]

            # if(torch.min(lh_patch) == 1 or torch.max(lh_patch) == 0): continue
            # if(torch.min(gt_patch) == 1 or torch.max(gt_patch) == 0): continue
            
            pd_gt, bcp_gt, dcp_gt = getCriticalPoints(gt_patch)
            pd_lh, bcp_lh, dcp_lh = getCriticalPoints(lh_patch)


            force_list, idx_holes_to_fix, idx_holes_to_remove = compute_dgm_force(pd_lh, pd_gt, pers_thresh=0)

            if (len(idx_holes_to_fix) > 0 or len(idx_holes_to_remove) > 0):

                for hole_indx in idx_holes_to_fix:

                    if (int(bcp_lh[hole_indx][0]) >= 0 and int(bcp_lh[hole_indx][0]) < likelihood.shape[0] and int(
                            bcp_lh[hole_indx][1]) >= 0 and int(bcp_lh[hole_indx][1]) < likelihood.shape[1]):
                        topo_cp_weight_map[y + int(bcp_lh[hole_indx][0]), x + int(
                            bcp_lh[hole_indx][1])] = 1  # push birth to 0 i.e. min birth prob or likelihood
                        topo_cp_ref_map[y + int(bcp_lh[hole_indx][0]), x + int(bcp_lh[hole_indx][1])] = 0
                    # if(y+int(dcp_lh[hole_indx][0]) < et_dmap.shape[2] and x+int(dcp_lh[hole_indx][1]) < et_dmap.shape[3]):
                    if (int(dcp_lh[hole_indx][0]) >= 0 and int(dcp_lh[hole_indx][0]) < likelihood.shape[
                        0] and int(dcp_lh[hole_indx][1]) >= 0 and int(dcp_lh[hole_indx][1]) <
                            likelihood.shape[1]):
                        topo_cp_weight_map[y + int(dcp_lh[hole_indx][0]), x + int(
                            dcp_lh[hole_indx][1])] = 1  # push death to 1 i.e. max death prob or likelihood
                        topo_cp_ref_map[y + int(dcp_lh[hole_indx][0]), x + int(dcp_lh[hole_indx][1])] = 1
                for hole_indx in idx_holes_to_remove:

                    if (int(bcp_lh[hole_indx][0]) >= 0 and int(bcp_lh[hole_indx][0]) < likelihood.shape[
                        0] and int(bcp_lh[hole_indx][1]) >= 0 and int(bcp_lh[hole_indx][1]) <
                            likelihood.shape[1]):
                        topo_cp_weight_map[y + int(bcp_lh[hole_indx][0]), x + int(
                            bcp_lh[hole_indx][1])] = 1  # push birth to death  # push to diagonal
                        # if(int(dcp_lh[hole_indx][0]) < likelihood.shape[0] and int(dcp_lh[hole_indx][1]) < likelihood.shape[1]):
                        if (int(dcp_lh[hole_indx][0]) >= 0 and int(dcp_lh[hole_indx][0]) < likelihood.shape[
                            0] and int(dcp_lh[hole_indx][1]) >= 0 and int(dcp_lh[hole_indx][1]) <
                                likelihood.shape[1]):
                            topo_cp_ref_map[y + int(bcp_lh[hole_indx][0]), x + int(bcp_lh[hole_indx][1])] = \
                                likelihood[int(dcp_lh[hole_indx][0]), int(dcp_lh[hole_indx][1])]
                        else:
                            topo_cp_ref_map[y + int(bcp_lh[hole_indx][0]), x + int(bcp_lh[hole_indx][1])] = 1
                            # if(y+int(dcp_lh[hole_indx][0]) < et_dmap.shape[2] and x+int(dcp_lh[hole_indx][1]) < et_dmap.shape[3]):
                    if (int(dcp_lh[hole_indx][0]) >= 0 and int(dcp_lh[hole_indx][0]) < likelihood.shape[
                        0] and int(dcp_lh[hole_indx][1]) >= 0 and int(dcp_lh[hole_indx][1]) <
                            likelihood.shape[1]):
                        topo_cp_weight_map[y + int(dcp_lh[hole_indx][0]), x + int(
                            dcp_lh[hole_indx][1])] = 1  # push death to birth # push to diagonal
                        # if(int(bcp_lh[hole_indx][0]) < likelihood.shape[0] and int(bcp_lh[hole_indx][1]) < likelihood.shape[1]):
                        if (int(bcp_lh[hole_indx][0]) >= 0 and int(bcp_lh[hole_indx][0]) < likelihood.shape[
                            0] and int(bcp_lh[hole_indx][1]) >= 0 and int(bcp_lh[hole_indx][1]) <
                                likelihood.shape[1]):
                            topo_cp_ref_map[y + int(dcp_lh[hole_indx][0]), x + int(dcp_lh[hole_indx][1])] = \
                                likelihood[int(bcp_lh[hole_indx][0]), int(bcp_lh[hole_indx][1])]
                        else:
                            topo_cp_ref_map[y + int(dcp_lh[hole_indx][0]), x + int(dcp_lh[hole_indx][1])] = 0

        # topo_cp_weight_map = torch.tensor(topo_cp_weight_map, dtype=torch.float).to(device)
        # topo_cp_ref_map = torch.tensor(topo_cp_ref_map, dtype=torch.float).to(device)
        loss_topo = (((likelihood * topo_cp_weight_map) - topo_cp_ref_map) ** 2).sum()

        return loss_topo


if __name__ == "__main__":
    gt = 1 - imread('gt.png')
    lh = 1 - imread('out.png')

    loss_topo = getTopoLoss(lh, gt)
    print(loss_topo)
    print('time %.3f'%(time.time()-t0));
