set FILE="C:\Program Files\VR\Delta3D_REL-2.2.0\examples\data\demoMap\StaticMeshes\TestTownLt.ive"
start dt-cluster2009.exe %FILE%

start dt-cluster2009.exe %FILE% 127.0.0.1 left
start dt-cluster2009.exe %FILE% 127.0.0.1 right
start dt-cluster2009.exe %FILE% 127.0.0.1 down
