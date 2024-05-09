#include"mjhtree.h"

static void MjhtreeInsert(struct mjhtree*tree, mjtindex index) {
	mjtindex node_next = tree->child[NUM_TILE_IN_HAND][0];
	if (node_next == NUM_TILE_IN_HAND) {
		tree->child[NUM_TILE_IN_HAND][0] = index;
		tree->parent[index] = NUM_TILE_IN_HAND;
		tree->child[index][0] = NUM_TILE_IN_HAND;
		tree->child[index][1] = NUM_TILE_IN_HAND;
		tree->color[index] = 1;
	}
	else {
		mjtindex node;
		{
			char lrside;
			do {
				node = node_next;
				lrside = tree->tiles[index] <= tree->tiles[node] ? 0 : 1;
				node_next = tree->child[node][lrside];
			} while (node_next != NUM_TILE_IN_HAND);
			tree->child[node][lrside] = index;
		}
		tree->parent[index] = node;
		tree->child[index][0] = NUM_TILE_IN_HAND;
		tree->child[index][1] = NUM_TILE_IN_HAND;
		tree->color[index] = 1;

		if (tree->color[node] == 1) {
			node_next = index;
			do {
				mjtindex node_grandpa = tree->parent[node];
				if (node_grandpa == NUM_TILE_IN_HAND) {
					tree->color[node] = 0;
					break;
				}
				else {
					char lrside = tree->child[node_grandpa][0] == node ? 0 : 1;
					mjtindex node_uncle = tree->child[node_grandpa][1 - lrside];
					if (tree->color[node_uncle] == 0) {
						if (node_next == tree->child[node][1 - lrside]) {
							tree->parent[tree->child[node_grandpa][lrside] = node_next] = node_grandpa;
							tree->parent[tree->child[node][1 - lrside] = tree->child[node_next][lrside]] = node;
							tree->parent[tree->child[node_next][lrside] = node] = node_next;

							node = node_next;
						}


						{
							mjtindex node_grandpaparent = tree->parent[node_grandpa];
							if (tree->child[node_grandpaparent][0] == node_grandpa)
								tree->child[node_grandpaparent][0] = node;
							else tree->child[node_grandpaparent][1] = node;
							tree->parent[node] = node_grandpaparent;
						}
						tree->parent[tree->child[node_grandpa][lrside] = tree->child[node][1 - lrside]] = node_grandpa;
						tree->parent[tree->child[node][1 - lrside] = node_grandpa] = node;

						tree->color[node_grandpa] = 1;
						tree->color[node] = 0;

						break;
					}
					else {
						tree->color[node] = 0;
						tree->color[node_uncle] = 0;
						tree->color[node_grandpa] = 1;

						node_next = node_grandpa;
						node = tree->parent[node_next];
					}
				}
			} while (node != NUM_TILE_IN_HAND);
		}
	}
}

void MjhtreeDelete(struct mjhtree*tree, mjtindex index) {
	{
		mjtindex node_parent = tree->parent[index];
		mjtindex node_left = tree->child[index][0];
		mjtindex node_right = tree->child[index][1];
		mjtindex*child_node_parent =
			tree->child[node_parent][0] == index ? tree->child[node_parent] :
			tree->child[node_parent] + 1;

		if (node_left != NUM_TILE_IN_HAND) {
			if (node_right != NUM_TILE_IN_HAND) {
				mjtindex node_child = tree->child[node_right][0];
				if (node_child != NUM_TILE_IN_HAND) {
					mjtindex node_sub = node_right, node_parent_sub;
					do {
						node_parent_sub = node_sub;
						node_sub = node_child;
						node_child = tree->child[node_sub][0];
					}
					while (node_child != NUM_TILE_IN_HAND);

					tree->parent[tree->child[index][1] = tree->child[node_parent_sub][0] = tree->child[node_sub][1]] = node_parent_sub;
					tree->parent[index] = node_parent_sub;
					{
						mjtcolor color_swap = tree->color[node_sub];
						tree->color[node_sub] = tree->color[index];
						tree->color[index] = color_swap;
					}

					*child_node_parent = node_sub;
					tree->parent[node_sub] = node_parent;
					tree->parent[tree->child[node_sub][0] = node_left] = node_sub;
					tree->parent[tree->child[node_sub][1] = node_right] = node_sub;
				}
				else {
					tree->child[index][1] = tree->child[node_right][1];
					tree->parent[index] = node_right;
					{
						mjtcolor color_swap = tree->color[node_right];
						tree->color[node_right] = tree->color[index];
						tree->color[index] = color_swap;
					}

					*child_node_parent = node_right;
					tree->parent[node_right] = node_parent;
					tree->parent[tree->child[node_right][0] = node_left] = node_right;
				}
			}
			else {
				*child_node_parent = node_left;
				tree->parent[node_left] = node_parent;
				tree->color[node_left] = 0;
				return;
			}
		}
		else {
			if (node_right != NUM_TILE_IN_HAND) {
				*child_node_parent = node_right;
				tree->parent[node_right] = node_parent;
				tree->color[node_right] = 0;
				return;
			}
			else {
				*child_node_parent = NUM_TILE_IN_HAND;
			}
		}
	}

	if (tree->color[index] == 0 && tree->child[index][1] == NUM_TILE_IN_HAND) {
		mjtindex node = NUM_TILE_IN_HAND;
		mjtindex node_parent = tree->parent[index];
		while (node_parent != NUM_TILE_IN_HAND) {
			char lrside = tree->child[node_parent][0] == node ? 0 : 1;
			mjtindex node_sibling = tree->child[node_parent][1 - lrside];
			mjtindex node_close = tree->child[node_sibling][lrside];
			mjtindex node_distant = tree->child[node_sibling][1 - lrside];
			mjtindex node_grandpa = tree->parent[node_parent];
			char lrside_parent = tree->child[node_grandpa][0] == node_parent ? 0 : 1;

			if (tree->color[node_sibling] == 1) {
				tree->parent[tree->child[node_grandpa][lrside_parent] = node_sibling] = node_grandpa;
				tree->parent[tree->child[node_sibling][lrside] = node_parent] = node_sibling;
				tree->parent[tree->child[node_parent][1 - lrside] = node_close] = node_parent;

				tree->color[node_sibling] = 0;
				tree->color[node_parent] = 1;
				
				node_sibling = node_close;
				node_close = tree->child[node_sibling][lrside];
				node_distant = tree->child[node_sibling][1 - lrside];
			}

			if (tree->color[node_close] > tree->color[node_distant]) {
				tree->parent[tree->child[node_parent][1 - lrside] = node_close] = node_parent;
				tree->parent[tree->child[node_sibling][lrside] = tree->child[node_close][1 - lrside]] = node_sibling;
				tree->parent[tree->child[node_close][1 - lrside] = node_sibling] = node_close;

				tree->color[node_close] = 0;
				tree->color[node_sibling] = 1;

				node_distant = node_sibling;
				node_sibling = node_close;
				node_close = tree->child[node_sibling][lrside];
			}

			if (tree->color[node_close] + tree->color[node_distant] == 0) {
				if (tree->color[node_parent] == 0) {
					tree->color[node_sibling] = 1;

					node = node_parent;
					node_parent = tree->parent[node];
				}
				else {
					tree->color[node_parent] = 0;
					tree->color[node_sibling] = 1;
					break;
				}
			}
			else {
				tree->parent[tree->child[node_grandpa][lrside_parent] = node_sibling] = node_grandpa;
				tree->parent[tree->child[node_parent][1 - lrside] = tree->child[node_sibling][lrside]] = node_parent;
				tree->parent[tree->child[node_sibling][lrside] = node_parent] = node_sibling;

				tree->color[node_sibling] = tree->color[node_parent];
				tree->color[node_parent] = 0;
				tree->color[node_distant] = 0;
				break;
			}
		}
	}
}

void MjhtreeInit(struct mjhtree*tree, mjtile const*tiles_new) {
	tree->tiles = tiles_new;
	tree->child[NUM_TILE_IN_HAND][0] = NUM_TILE_IN_HAND;
	tree->child[NUM_TILE_IN_HAND][1] = NUM_TILE_IN_HAND;
	tree->color[NUM_TILE_IN_HAND] = 0;
	for (int index = 0; index < NUM_TILE_IN_HAND; ++index) {
		MjhtreeInsert(tree, index);
	}
}

mjtindex MjhtreeFindTile(struct mjhtree const*tree, mjtile t) {
	mjtindex node = tree->child[NUM_TILE_IN_HAND][0];
	while (node != NUM_TILE_IN_HAND) {
		if (t < tree->tiles[node])
			node = tree->child[node][0];
		else if (t > tree->tiles[node])
			node = tree->child[node][1];
		else return node;
	}
	return NUM_TILE_IN_HAND;
}

mjtindex MjhtreeFindKind(struct mjhtree const*tree, mjkind k) {
	mjkind const kk = MjUnbuffKind(k);
	mjtindex node = tree->child[NUM_TILE_IN_HAND][0];
	while (node != NUM_TILE_IN_HAND) {
		if (kk * NUM_TILE_EACHKIND + (NUM_TILE_EACHKIND - 1) < tree->tiles[node])
			node = tree->child[node][0];
		else if (kk * NUM_TILE_EACHKIND > tree->tiles[node])
			node = tree->child[node][1];
		else return node;
	}
	return NUM_TILE_IN_HAND;
}

void MjhtreeUpdate(struct mjhtree*tree, mjtindex index) {
	MjhtreeDelete(tree, index);
	MjhtreeInsert(tree, index);
}

static void Recur_MjhtreeIsConsistent(struct mjhtree*tree, mjhand h, mjtindex node) {
	if (node != NUM_TILE_IN_HAND) {
		h[MjBuffKind(tree->tiles[node] / NUM_TILE_EACHKIND)] += 1;
		Recur_MjhtreeIsConsistent(tree, h, tree->child[node][0]);
		Recur_MjhtreeIsConsistent(tree, h, tree->child[node][1]);
	}
}

char MjhtreeIsConsistent(struct mjhtree*tree, mjhand const hand) {
	mjhand h = { 0 };
	Recur_MjhtreeIsConsistent(tree, h, tree->child[NUM_TILE_IN_HAND][0]);
	for (int i = 0; i < sizeof(h) / sizeof(*h); ++i) {
		if (h[i] != hand[i]) return 0;
	}
	return 1;
}