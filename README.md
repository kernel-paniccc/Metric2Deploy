# Meric2Deploy
Full deployment cycle for the metrics application

## Fast start

### 1) k3s cluster deployment

```
cd Vagrant-k3s-cluster

vagrant up --provider=libvirt

ansible-playbook -i ansible/inventory.ini ansible/k3s-claster.yml

```

### 2) app build

```
cd src

docker build -t meric2deploy-cpp:latest .
```

### 3) app deploy to k3s

```
kubectl apply -f src/k8s/deployment.yaml
kubectl apply -f src/k8s/service.yaml
```
