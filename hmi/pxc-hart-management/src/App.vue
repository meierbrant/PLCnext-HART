<template>
  <div id="app">
    <MobileNavBar/>
    <div class="d-flex">
      <div class="d-lg-block d-none" style="min-width: 16rem; height: 1px;"></div>
      <DesktopNavBar style="width: 16rem;"/>
      <router-view style="flex: 1"/>
    </div>
  </div>
</template>

<script>
// @ is an alias to /src
import DesktopNavBar from '@/components/DesktopNavBar.vue'
import MobileNavBar from '@/components/MobileNavBar.vue'

export default {
  name: 'app',
  components: {
    DesktopNavBar,
    MobileNavBar
  }
}
</script>

<style lang="scss">
@import './assets/scss/app.scss';

#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

#nav {
  padding: 30px;

  a {
    font-weight: bold;
    color: #2c3e50;

    &.router-link-exact-active {
      color: #42b983;
    }
  }
}
</style>
