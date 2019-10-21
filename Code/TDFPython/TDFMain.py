import numpy as np
import matplotlib.pyplot as plt
import math


def compute_persistence_2DImg_1DHom(f):
    """
    compute persistence diagram in a 2D function (can be N-dim) and critical pts
    only generate 1D homology dots and critical points
    """
    assert len(f.shape) == 2  # f has to be 2D function
    dim = 2

    # pad the function with a few pixels of minimum values
    # this way one can compute the 1D topology as loops
    # remember to transform back to the original coordinates when finished
    padwidth = 2
    padvalue = min(f.min(), 0.0)
    f_padded = np.pad(f, padwidth, 'constant', constant_values=padvalue)

    # call persistence code to compute diagrams
    # loads PersistencePython.so (compiled from C++); should be in current dir
    from PersistencePython import cubePers
    persistence_result = cubePers(np.reshape(
        f_padded, f_padded.size).tolist(), list(f_padded.shape), 0.001)

    # only take 1-dim topology, first column of persistence_result is dimension
    persistence_result_filtered = np.array(filter(lambda x: x[0] == 1,
                                                  persistence_result))

    # persistence diagram (second and third columns are coordinates)
    dgm = persistence_result_filtered[:, 1:3]

    # critical points
    birth_cp_list = persistence_result_filtered[:, 4:4 + dim]
    death_cp_list = persistence_result_filtered[:, 4 + dim:]

    # when mapping back, shift critical points back to the original coordinates
    birth_cp_list = birth_cp_list - padwidth
    death_cp_list = death_cp_list - padwidth

    return dgm, birth_cp_list, death_cp_list


def compute_dgm_force(lh_dgm, gt_dgm):
    # get persistence list from both diagrams
    lh_pers = lh_dgm[:, 1] - lh_dgm[:, 0]
    gt_pers = gt_dgm[:, 1] - gt_dgm[:, 0]

    # more lh dots than gt dots
    assert lh_pers.size > gt_pers.size

    # check to ensure that all gt dots have persistence 1
    tmp = gt_pers > 0.999
    assert tmp.sum() == gt_pers.size

    gt_n_holes = gt_pers.size  # number of holes in gt

    # get "perfect holes" - holes which do not need to be fixed, i.e., find top
    # lh_n_holes_perfect indices
    # check to ensure that at least one dot has persistence 1; it is the hole
    # formed by the padded boundary
    # if no hole is ~1 (ie >.999) then just take all holes with max values
    tmp = lh_pers > 0.999  # old: assert tmp.sum() >= 1
    if tmp.sum >= 1:
        # n_holes_to_fix = gt_n_holes - lh_n_holes_perfect
        lh_n_holes_perfect = tmp.sum()
        idx_holes_perfect = np.argpartition(lh_pers, -lh_n_holes_perfect)[
                            -lh_n_holes_perfect:]
    else:
        idx_holes_perfect = np.where(lh_pers == lh_pers.max())[0]

    # find top gt_n_holes indices
    idx_holes_to_fix_or_perfect = np.argpartition(lh_pers, -gt_n_holes)[
                                  -gt_n_holes:]

    # the difference is holes to be fixed to perfect
    idx_holes_to_fix = list(
        set(idx_holes_to_fix_or_perfect) - set(idx_holes_perfect))

    # remaining holes are all to be removed
    idx_holes_to_remove = list(
        set(range(lh_pers.size)) - set(idx_holes_to_fix_or_perfect))

    # only select the ones whose persistence is large enough
    # set a threshold to remove meaningless persistence dots
    # TODO values below this are small dents so dont fix them; tune this value?
    pers_thd = 0.03
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

    return force_list, idx_holes_to_fix, idx_holes_to_remove


def compute_topological_loss(lh_dgm, gt_dgm):
    """
    compute persistence loss
    """
    force_list, idx_holes_to_fix, idx_holes_to_remove = \
        compute_dgm_force(lh_dgm, gt_dgm)
    loss = 0.0
    for idx in idx_holes_to_fix:
        loss = loss + force_list[idx, 0] ** 2 + force_list[idx, 1] ** 2
    for idx in idx_holes_to_remove:
        loss = loss + force_list[idx, 0] ** 2 + force_list[idx, 1] ** 2
    return loss


def compute_topological_grad(lh_dgm, lh_bcp, lh_dcp, gt_dgm):
    """
    compute topological gradient
    """
    force_list, idx_holes_to_fix, idx_holes_to_remove = \
        compute_dgm_force(lh_dgm, gt_dgm)

    # each birth/death crit pt of a persistence dot to move corresponds to a row
    # each row has 3 values: x, y coordinates, and the force (increase/decrease)
    topo_grad = np.zeros(
        [2 * (len(idx_holes_to_fix) + len(idx_holes_to_remove)), 3])

    counter = 0
    for idx in idx_holes_to_fix:
        topo_grad[counter] = [lh_bcp[idx, 1], lh_bcp[idx, 0], force_list[idx, 0]]
        counter = counter + 1
        topo_grad[counter] = [lh_dcp[idx, 1], lh_dcp[idx, 0], force_list[idx, 1]]
        counter = counter + 1
    for idx in idx_holes_to_remove:
        topo_grad[counter] = [lh_bcp[idx, 1], lh_bcp[idx, 0], force_list[idx, 0]]
        counter = counter + 1
        topo_grad[counter] = [lh_dcp[idx, 1], lh_dcp[idx, 0], force_list[idx, 1]]
        counter = counter + 1

    topo_grad[:, 2] = topo_grad[:, 2] * -2
    return topo_grad


def save_pers_dgms(fig_fname, lh_dgm, lh_dname, gt_dgm, gt_dname):
    """
    draw both diagrams, with respect names, save the figure to fig_fname
    """
    plt.figure('Diagrams')
    plt.clf()
    plt.plot([0, 1], [0, 1], 'k-')

    force_list, idx_holes_to_fix, idx_holes_to_remove = \
        compute_dgm_force(lh_dgm, gt_dgm)

    plt.scatter([lh_dgm[:, 0]], [lh_dgm[:, 1]], color='green', label=lh_dname)

    for idx in idx_holes_to_fix:
        plt.arrow(lh_dgm[idx, 0], lh_dgm[idx, 1], force_list[idx, 0],
                  force_list[idx, 1], head_width=0.02, head_length=0.03,
                  color='r')

    for idx in idx_holes_to_remove:
        plt.arrow(lh_dgm[idx, 0], lh_dgm[idx, 1], force_list[idx, 0],
                  force_list[idx, 1], head_width=0.02, head_length=0.03,
                  color='c')

    # add small noise to gt persistent dots, so they don't overlap too much
    gt_dgm = gt_dgm + 0.05 * (np.random.random_sample(gt_dgm.shape) - 0.5)

    plt.scatter([gt_dgm[:, 0]], [gt_dgm[:, 1]], color='blue',
                label=gt_dname + ' jittered')
    plt.ylabel('Death Time')
    plt.xlabel('Birth Time')
    plt.title('Persistence Diagrams')
    plt.axis('equal')
    plt.legend()
    plt.savefig(fig_fname)


def draw_critical_pts(fig_fname, lh_img, lh_dgm, lh_bcp, lh_dcp, gt_dgm):
    """
    draw critical points that needs to be pushed up/down
    """
    plt.figure('Critical Pts to Fix/Remove')
    plt.clf()
    plt.imshow(lh_img, 'gray')

    force_list, idx_holes_to_fix, idx_holes_to_remove = \
        compute_dgm_force(lh_dgm, gt_dgm)

    for idx in idx_holes_to_fix:
        plt.scatter(lh_bcp[idx, 1], lh_bcp[idx, 0], marker='v', color='r',
                    label='birth, fix, ' + str(force_list[idx, 0]))
        plt.scatter(lh_dcp[idx, 1], lh_dcp[idx, 0], marker='o', color='r',
                    label='death, fix, ' + str(force_list[idx, 1]))

    for idx in idx_holes_to_remove:
        plt.scatter(lh_bcp[idx, 1], lh_bcp[idx, 0], marker='v', color='c',
                    label='birth, remove, ' + str(force_list[idx, 0]))
        plt.scatter(lh_dcp[idx, 1], lh_dcp[idx, 0], marker='o', color='c',
                    label='death, remove, ' + str(force_list[idx, 1]))

    plt.title('Persistence Pts to Fix/Remove')
    plt.axis('equal')
    # plt.legend(loc=5,bbox_to_anchor=(2, 0.5))
    plt.legend(bbox_to_anchor=(1.1, 0.7), loc=5, borderaxespad=0.)
    plt.tight_layout()
    plt.savefig(fig_fname)


if __name__ == "__main__":
    ## example usage

    # load lh, gt from likelihood.npy
    result_folder = '../../Results/'
    likelihood_fname = '../../Data/predicted_1.npy'
    tmpX = np.load(likelihood_fname)
    #likelihood = tmpX[1, :, :, 1] # take one slice
    likelihood = tmpX
    likelihood = likelihood[: ,:]  # crop a piece
    groundtruth = likelihood > 0.9 # approx gt using likelihood

    # save the loaded (and modified) lh, gt
    np.save(result_folder + 'likelihood_cropped.npy', likelihood)
    plt.imsave(result_folder + 'likelihood_cropped.png', likelihood, cmap='gray')
    np.save(result_folder + 'groundtruth_cropped.npy', groundtruth)
    plt.imsave(result_folder + 'groundtruth_cropped.png', groundtruth,
               cmap='gray')

    # compute persistence
    pd_lh, bcp_lh, dcp_lh = compute_persistence_2DImg_1DHom(likelihood)
    pd_gt, bcp_gt, dcp_gt = compute_persistence_2DImg_1DHom(groundtruth)

    # draw the persistence diagrams of both
    dgm_fig_fname = result_folder + 'PersDgms.png'
    save_pers_dgms(dgm_fig_fname, pd_lh, 'Likelihood', pd_gt, 'Ground Truth')

    # draw critical pts
    cpts_fig_fname = result_folder + 'PersCpts.png'
    draw_critical_pts(cpts_fig_fname, likelihood, pd_lh, bcp_lh, dcp_lh, pd_gt)

    # compute loss (scalar) and gradient of loss (forces at x,y locations in the
    # image which corresp. to crit pts from the persistence computation)
    topo_loss = compute_topological_loss(pd_lh, pd_gt)
    topo_grad = compute_topological_grad(pd_lh, bcp_lh, dcp_lh, pd_gt)
    print ('loss = ', topo_loss)
    print ('grad = ', topo_grad)
