<template>
<b-navbar id="mobile-navbar" class="d-lg-none" toggleable="lg" type="light" variant="primary">
    <b-navbar-brand to="/"><img src="../assets/img/pxc_logo_plain_white.png"></b-navbar-brand>

    <b-navbar-toggle target="nav-collapse"></b-navbar-toggle>

    <b-collapse id="nav-collapse" is-nav>
        <b-navbar-nav>
            <b-nav-item to="/">Gateways</b-nav-item>
        </b-navbar-nav>
    </b-collapse>
</b-navbar>
</template>

<style scoped lang="scss">
@import '../assets/scss/_vars.scss';

#mobile-navbar {
    font-size: 2em;

    .nav-link {
        color: #fff;

        &:active {
            background-color: darken($primary-color, 10);
        }
    }
}
</style>