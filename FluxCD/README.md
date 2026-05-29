# Minimal FluxCD

This setup leaves Flux as a pure Git sync layer.

## What it does

1. GitHub Actions builds and pushes a new image to `ghcr.io/kernel-paniccc/metric2deploy`.
2. The workflow tags each build with:
   - `latest`
   - `0.0.<github_run_number>`
   - `sha-<commit>`
3. The workflow updates `k3s-manifest/deployment.yaml` to the new `0.0.<github_run_number>` tag and commits that change back to `main`.
4. Flux watches the Git repository and applies `k3s-manifest/` to the cluster.
5. Kubernetes rolls out the updated deployment.

## Apply

```bash
kubectl apply -f FluxCD/*.yaml
```
