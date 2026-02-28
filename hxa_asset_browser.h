


typedef void HxAAssetBrowserSession;

extern HxAAssetBrowserSession *hxa_asset_browser_create(uint resolution, void (*custom_render)(HXANode *node, void *fbo, float *center, float scale, uint resolution, void *user_pointer), void *user_pointer);
extern void						hxa_asset_browser_destroy(HxAAssetBrowserSession *session);
extern void						hxa_asset_browser_add_asset(HxAAssetBrowserSession *session, HXAFile *file, char *path, char *name);
extern void						hxa_asset_browser_add_directory(HxAAssetBrowserSession *session, char *path, HxAAssetBrowserSession *unless_in_session);
extern void						hxa_asset_browser_remove_asset(HxAAssetBrowserSession *session, HXAFile *file);
extern char						*hxa_asset_browser_name_get(HxAAssetBrowserSession *session, HXAFile *file);
extern char						*hxa_asset_browser_path_get(HxAAssetBrowserSession *session, HXAFile *file);
extern HXAFile					*hxa_asset_browser_get_by_name(HxAAssetBrowserSession *session, char *name);

extern boolean					hxa_asset_browser_asset_return(HxAAssetBrowserSession *session, HXAFile *file, boolean save);
extern HXAFile					*hxa_asset_browser_draw(BInputState *input, HxAAssetBrowserSession *session, float top, float bottom, float size, boolean create_new);


extern uint						hxa_asset_browser_asset_count_get(HxAAssetBrowserSession *session);
extern uint						hxa_asset_browser_asset_lookup_by_name(HxAAssetBrowserSession *session, char *name);
extern char						*hxa_asset_browser_asset_name_get(HxAAssetBrowserSession *session, uint id);
extern uint						hxa_asset_browser_asset_texture_id_get(HxAAssetBrowserSession *session, uint id);
extern char						*hxa_asset_browser_asset_path_get(HxAAssetBrowserSession *session, uint id);
extern float					hxa_asset_browser_asset_size_get(HxAAssetBrowserSession *session, uint id);
extern HXAFile					*hxa_asset_browser_asset_file_get(HxAAssetBrowserSession *session, uint id);