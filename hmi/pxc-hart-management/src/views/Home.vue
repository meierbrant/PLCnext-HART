<template>
<main role="main" class="text-center">
  <h1 class="cover-heading">Phoenix Contact HART Management System</h1>
  <p class="lead">Easily view and manage your HART device network.</p>
  <p class="lead">
    <HartGateways />
  </p>
</main>
</template>

<script>
// @ is an alias to /src
import HartGateways from '@/components/HartGateways.vue'

export default {
  name: 'home',
  components: {
    HartGateways
  }
}
</script>

<style lang="scss" scoped>
.container {
  display: flex;
  flex-direction: column;
  align-items: center;
  height: 100%;
  min-height: 100%;
}

main {
  display: flex;
  flex: 1;
  flex-flow: column;
  align-items: center;
  max-width: 800px;
  margin: 2em auto;
}
</style>