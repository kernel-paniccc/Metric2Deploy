# Metric2Deploy

## Fast Start

### Requirments

- `vagrant`
- `vagrant-libvirt`
- `libvirt`
- `ansible`
- `kubectl`
- `kubectx`
- `kubens`

### 1. Поднимаем виртуалки

```bash
cd ~/Projects-git/Metric2Deploy/Vagrant-k3s-cluster
vagrant up --provider=libvirt
vagrant status
```

### 2. Раскатывает k3s и тянем kubeconfig через Ansible-роль

```bash
ansible-playbook -i ansible/inventory.ini ansible/k3s-claster.yml -v
```

### 3. Сетапим саб-конфиг в глобальный KUBECONFIG через kubectx


```bash
unset KUBECONFIG
kubectl config get-contexts
kubectx metric2deploy
```