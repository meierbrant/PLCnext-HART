<template>
<b-navbar id="mobile-navbar" class="d-lg-none" toggleable="lg" type="light" variant="primary">
    <b-navbar-brand to="/"><img src="../assets/img/pxc_logo_plain.png"></b-navbar-brand>

    <b-navbar-toggle target="nav-collapse"></b-navbar-toggle>

    <b-collapse id="nav-collapse" is-nav>
        <b-navbar-nav>
            <b-nav-item to="/">Gateways</b-nav-item>
        </b-navbar-nav>
    </b-collapse>
</b-navbar>
</template>

<style scoped lang="scss">
#mobile-navbar {
    font-size: 2em;
}
</style>